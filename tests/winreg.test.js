var assert = require("assert");
var reg = require("..");

describe("winreg", function() {
  describe("module const", function() {
    it("const values", function() {
      assert.ok(reg.KEY_READ > 0);
      assert.ok(reg.HKEY_CURRENT_USER > 0);
      assert.ok(reg.HKEY_LOCAL_MACHINE > 0);
      assert.ok(reg.HKEY_CLASSES_ROOT > 0);
    });
  });

  describe("new RegKey", function(){
    it("not valid", function() {
      var k = new reg.RegKey();
      assert.ok(!k.isValid);
    });

    it("open" , function() {
      var k = new reg.RegKey();
      assert.ok(!k.isValid);
      k.open(reg.HKEY_CURRENT_USER, "Software\\Python\\PythonCore\\3.6")
      assert(k.isValid);
      assert.equal(k.getString("DisplayName"), "Python 3.6 (64-bit)");
    });

    it("open and close" , function() {
      var k = new reg.RegKey();
      assert.ok(!k.isValid);
      const r = k.open(reg.HKEY_CURRENT_USER, "Software\\Python\\PythonCore\\3.6")      
      assert(r);
      assert(k.isValid);
      assert(r.getString("aaaaa", "123"), "123");
      k.close();
      assert.ok(!k.isValid);
    });

    it("open no error" , function() {
      var k = new reg.RegKey();
      assert.ok(!k.isValid);
      const r = k.open(reg.HKEY_CURRENT_USER, "Software\\Python\\xxxx")
      assert(!r);
      assert.ok(!k.isValid);      
      k.close();
      assert.ok(!k.isValid);
    });


    it("get value type", function() {
      var k = new reg.RegKey();
      k.open(reg.HKEY_CURRENT_USER, "Environment", reg.KEY_READ);
      assert.ok(k.isValid);
      assert.equal(k.getValueType("NVM_HOMExxx"), 0); // REG_NONE   0
      assert.equal(k.getValueType("NVM_HOME"), 2);    // REG_EXPAND_SZ 2
      assert.equal(k.getValueType("VCPKG_ROOT"), 1);  // REG_SZ 1
    })
  });


  describe("new RegKey with arguments", function(){
    it("getString", function() {
      const k= new reg.RegKey(reg.HKEY_CURRENT_USER, "Software\\Python\\PythonCore\\3.6")
      assert.ok(k.isValid);
      assert.equal(k.getString("DisplayName"), "Python 3.6 (64-bit)");
      assert.equal(k.getString("NonExists", "Default"), "Default");
    });

    it("getDword", function() {
      const k= new reg.RegKey(reg.HKEY_CURRENT_USER, "Console", reg.KEY_READ)
      assert.ok(k.isValid);
      assert.equal(k.getDword("CodePage"), 65001);
      assert.equal(k.getDword("NonExists", 12345), 12345);
    });

    it("get Throw", function () {
        var k = new reg.RegKey(reg.HKEY_CURRENT_USER, "Software\\Python\\PythonCore\\3.6", 0x80000000);
        assert.throws(() => k.getString("NonExists"), /RegGetValue failed/);
        assert.throws(() => k.getDword("NonExists"), /RegGetValue failed/);
        assert.throws(() => k.getDword("DisplayName"), /RegGetValue failed/);
    });
  });

  describe("enumSubKey", function(){
    it("valid", function() {
      const k = new reg.RegKey(reg.HKEY_CURRENT_USER, "Software\\Google", 0x80000000);
      assert.ok(k.isValid);
      let keys = k.enumSubKeys();
      assert.ok(keys.includes("Chrome"));
    });
  });

  describe("enumValues", function(){
    it("valid", function() {
      const k = new reg.RegKey(reg.HKEY_CURRENT_USER, "Environment", 0x80000000);
      assert.ok(k.isValid);
      let keys = k.enumValues();
      assert.equal(keys.TEMP, 2);
    });
  });



  describe("createKey", function(){
    it("create subkey", function() {
      const k = new reg.RegKey(reg.HKEY_CURRENT_USER, "ATA\\test");
      assert.ok(k.isValid);
      k.setString("name", "中文");
      assert.equal(k.getString("name"), "中文");
      k.deleteValue("name");
      assert.throws(() => k.getString("name"), /RegGetValue failed/);
    });

    it("delete subkey", function() {
      const k = new reg.RegKey(reg.HKEY_CURRENT_USER, "ATA");
      k.deleteKey("test", reg.KEY_WOW64_64KEY);
    });
  });



  describe("global-delete", function() {
    // Applies only to tests in this describe block
    beforeEach(() => {
      var k = new reg.RegKey();
      k.create(reg.HKEY_CURRENT_USER, "Software/ata/test", reg.KEY_WRITE | reg.KEY_READ);
      assert.ok(k.isValid);

      k.setString("value_str", "hello");
      k.setDword("value_dword", 12345);
      assert.ok(k.getString("value_str") === "hello");
      assert.ok(k.getDword("value_dword") === 12345);
      k.close();
    });

    it("queryValue", function() {
      var v =  reg.queryValue(reg.HKEY_CURRENT_USER, "Software/ata/test", "value_str");
      assert.ok(v === "hello");
      v =  reg.queryValue(reg.HKEY_CURRENT_USER, "Software/ata/test", "value_dword");
      assert.ok(v === 12345);

      v =  reg.queryValue(reg.HKEY_CURRENT_USER, "Software/ata/testxx", "value_str");
      assert.ok(v === null);

      v =  reg.queryValue(reg.HKEY_CURRENT_USER, "Software/ata/test", "value_str2");
      assert.ok(v === null);

    });


    it("delete", function() {
      var v =  reg.queryValue(reg.HKEY_CURRENT_USER, "Software/ata/test", "value_str");
      assert.ok(v === "hello");

      v = reg.delete(reg.HKEY_CURRENT_USER, "Software/ata/test", "value_str");
      assert.ok(v);

      v =  reg.queryValue(reg.HKEY_CURRENT_USER, "Software/ata/test", "value_str");
      assert.ok(v === null);

      v = reg.delete(reg.HKEY_CURRENT_USER, "Software/ata/test");
      assert.ok(v);

      v =  reg.queryValue(reg.HKEY_CURRENT_USER, "Software/ata/test", "value_dword");
      assert.ok(v === null);
    });

    it("set", function() {
      v = reg.delete(reg.HKEY_CURRENT_USER, "Software/ata/test");
      assert.ok(v);

      var v =  reg.set(reg.HKEY_CURRENT_USER, "Software/ata/test/sub1/sub2", "value_str", "hello2");
      assert.ok(v);
      var v =  reg.set(reg.HKEY_CURRENT_USER, "Software/ata/test/sub1/sub2", "value_dword", 789);
      assert.ok(v);

      v =  reg.queryValue(reg.HKEY_CURRENT_USER, "Software/ata/test/sub1/sub2", "value_str");
      assert.ok(v === "hello2");

      v =  reg.queryValue(reg.HKEY_CURRENT_USER, "Software/ata/test/sub1/sub2", "value_dword");
      assert.ok(v === 789);

      v = reg.delete(reg.HKEY_CURRENT_USER, "Software/ata/test");
      assert.ok(v);
    });
  });
});

