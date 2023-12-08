const fs = require('fs');
const reg = require('.');

function getOSLoaders() {
  const keys_element = {
    "12000004" : "description",
    "12000002" : "path",
    "22000002" : "systemroot",
    "22000011" : "kernel",
  };

  const osloaders = [];

  try {
    const k = new reg.RegKey();

    // Windows Boot Manager
    // default osloader
    const defaultLoader = reg.queryValue(
        reg.HKEY_LOCAL_MACHINE,
        "BCD00000000\\Objects\\{9dea862c-5cdd-4e70-acc1-f32b344d4795}\\Elements\\23000003",
        "Element");

    // enumerate osloader
    k.open(reg.HKEY_LOCAL_MACHINE, "BCD00000000\\Objects", reg.KEY_READ);
    const keys = k.enumSubKeys();
    for (let i = 0; i < keys.length; i++) {
      const type =
          reg.queryValue(k.handle(), keys[i] + "\\Description", "Type") || 0;
      if (type === 0x10200003) { // osloader
        const info = {id : keys[i], default : keys[i] === defaultLoader};

        const ksub = new reg.RegKey();
        Object.keys(keys_element).forEach(name => {
          if (ksub.open(k.handle(), keys[i] + "\\Elements\\" + name,
                        reg.KEY_READ)) {
            info[keys_element[name]] =
                ksub.getString("Element", "").replaceAll('\0', '');
          }
        })
        ksub.close();
        osloaders.push(info);
      }
    }
    k.close();
  } catch (e) {
    console.log("error:", e, e.code);
  }
  return osloaders;
}

console.log(getOSLoaders());
