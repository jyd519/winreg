 var reg = require(`D:\\ata\\joytest\\addon\\winreg\\build\\Debug\\winreg.node`);
 var k = new reg.RegKey();
try {
    k.open(reg.HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control", reg.KEY_READ);
   console.log( k.getDword("BootDriverFlags"));
   console.log( k.getString("SystemBootDevice"));
   console.log( k.enumValues("OptionValue"));
   console.log( k.getDword("OptionValue"));
} catch( e) {
console.log(e, e.code);
}
