var  pp= require('child_process');
//x = pp.execSync(['wmic', 'SYSTEMCOMPUTER', 'get',  'bootupstate'], { encoding: 'utf8'});
x = pp.execSync('wmic.exe COMPUTERSYSTEM get bootupstate', { encoding: 'utf8'});


console.log(x.replace(/\r|\n/g, ' '));