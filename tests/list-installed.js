const fs = require('fs');
const reg = require('..');

function safe_reg(k, cb) {
  try {
    cb(k);
    return true;
  } catch (e) {
    return e;
  } finally {
    k.close();
  }
}

function getInstalledApplications() {
  const products = [];

  const k = new reg.RegKey();
  const readInstallation = (relpath) => {
    safe_reg(new reg.RegKey(), (ksub)=> {
      if (!ksub.open(k.handle(), relpath)) {
        return;
      }

      const values = ksub.enumValues();
      if ('DisplayName' in values) {
        const info = {};
        for (const n of Object.keys(values)) {
          if (values[n] == 1) {
            const value = ksub.getString(n)
            if (value) {
              info[n] = value;
            }
          }
        }
        products.push(info);
      }
    })
  };

  try {
    if (k.open(reg.HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall")) {
      const keys = k.enumSubKeys();
      for (let i =0; i<keys.length; i++){
        readInstallation(keys[i]);
      }
    }

    if (k.open(reg.HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", reg.KEY_WOW64_32KEY |reg.KEY_READ)) {
      const keys = k.enumSubKeys();
      for (let i =0; i<keys.length; i++){
        readInstallation(keys[i]);
      }
    }
    k.close();
  } catch (e) {
    /* handle error */
    k.close();
  }

  return products;
}

const products = getInstalledApplications()
console.log(JSON.stringify(products, "", " "));
fs.writeFileSync("products2.json", JSON.stringify(products, "", " "))

