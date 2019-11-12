 var reg = require(`D:\\ata\\joytest\\addon\\winreg\\build\\Debug\\winreg.node`);
  console.log("1");
 var k = new reg.RegKey();
  console.log("2", reg.HKEY_LOCAL_MACHINE, reg.KEY_READ);
try {
  k.open(reg.HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\SafeBoot\\Option1", reg.KEY_READ);
  console.log("3");
   console.log('2:', k.getDword("OptionValue"));
} catch( e) {
console.log(e);
}
