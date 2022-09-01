const arch = process.arch;
module.exports = require(`./${arch}/winreg.node`);
