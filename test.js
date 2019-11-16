var myModule;

function require_hook(name) {
  console.log('require', name);
  if (name.endsWith('winreg.node')) {
    return `D:\\ata\\joytest\\addon\\winreg\\build\\release\\winreg.node`;
    }
  return name;
}

var Module = require('module');
var _require = Module.prototype.require;
Module.prototype.require = function reallyNeedRequire(name, options) {
    options = options || {};
    
    name = require_hook(name);
    var nameToLoad = Module._resolveFilename(name, this);
    if (options.bust) {
        delete require.cache[nameToLoad];
    }
    console.log('require',nameToLoad);
    return _require.call(this, nameToLoad);
};


if (process.env.DEBUG) {
    myModule = require('./build/Debug/winreg.node')
} else {
    myModule = require('./build/Release/winreg.node')
}

var chai = require('chai');
var assert = chai.assert;

describe("winreg", function() {
  describe("module const", function() {
    it("const values", function() {
      assert.ok(myModule.KEY_READ > 0);
      assert.ok(myModule.HKEY_CURRENT_USER > 0);
      assert.ok(myModule.HKEY_LOCAL_MACHINE > 0);
      assert.ok(myModule.HKEY_CLASSES_ROOT > 0);
    });
  });

  describe("new RegKey", function(){
    it("not valid", function() {
      var k = new myModule.RegKey();
      assert.ok(!k.isValid);
    });

    it("open" , function() {
      var k = new myModule.RegKey();
      assert.ok(!k.isValid);
      k.open(myModule.HKEY_CURRENT_USER, "Software\\Python\\PythonCore\\3.6")
      assert(k.isValid);
      assert.equal(k.getString("DisplayName"), "Python 3.6 (64-bit)");
    });

    it("open and close" , function() {
      var k = new myModule.RegKey();
      assert.ok(!k.isValid);
      const r = k.open(myModule.HKEY_CURRENT_USER, "Software\\Python\\PythonCore\\3.6")      
      assert(r);
      assert(k.isValid);
      assert(r.getString("aaaaa", "123"), "123");
      k.close();
      assert.ok(!k.isValid);
    });

    it("open no error" , function() {
      var k = new myModule.RegKey();
      assert.ok(!k.isValid);
      const r = k.open(myModule.HKEY_CURRENT_USER, "Software\\Python\\xxxx")
      assert(!r);
      assert.ok(!k.isValid);      
      k.close();
      assert.ok(!k.isValid);
    });
  });


  describe("new RegKey with arguments", function(){
    it("getString", function() {
      const k= new myModule.RegKey(myModule.HKEY_CURRENT_USER, "Software\\Python\\PythonCore\\3.6")
      assert.ok(k.isValid);
      assert.equal(k.getString("DisplayName"), "Python 3.6 (64-bit)");
    });

    it("getDword", function() {
      const k= new myModule.RegKey(myModule.HKEY_CURRENT_USER, "Console", myModule.KEY_READ)
      assert.ok(k.isValid);
      assert.equal(k.getDword("CodePage"), 65001);
    });

    it("get Throw", function () {
        var k = new myModule.RegKey(myModule.HKEY_CURRENT_USER, "Software\\Python\\PythonCore\\3.6", 0x80000000);
        assert.throws(() => k.getString("DisplayName2"), /RegGetValue failed/);
    });
  });

  describe("enumSubKey", function(){
    it("valid", function() {
      const k = new myModule.RegKey(myModule.HKEY_CURRENT_USER, "Software\\Google", 0x80000000);
      let keys = k.enumSubKeys();
      assert.deepEqual(keys, ['Chrome', 'Software Removal Tool', 'Update']);
    });
  });

  describe("enumValues", function(){
    it("valid", function() {
      const k = new myModule.RegKey(myModule.HKEY_CURRENT_USER, "Environment", 0x80000000);
      let keys = k.enumValues();
      assert.deepInclude(keys, { "name": "ATA_ATOM_ENV", "type": 1} );
    });
  });



  describe("createKey", function(){
    it("create subkey", function() {
      const k = new myModule.RegKey(myModule.HKEY_CURRENT_USER, "ATA\\test");
      k.setString("name", "中文");
      assert.equal(k.getString("name"), "中文");

      k.deleteValue("name");
      assert.throws(() => k.getString("name"), /RegGetValue failed/);

    });
    it("delete subkey", function() {
      const k = new myModule.RegKey(myModule.HKEY_CURRENT_USER, "ATA");
      k.deleteKey("test", myModule.KEY_WOW64_64KEY);
    });
  });
});


