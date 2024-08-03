const fs = require('fs')
const path = require('path')

fs.readdir(path.resolve(),function(err,files)
{
    var result = {};
    files.forEach(element => {
        element.split('.').forEach(name => {
            if(name.indexOf('exe')!= -1)
            {
                if (!result[name.split('_')[1]])
                {
                    result[name.split('_')[1]] = [];
                }
                result[name.split('_')[1]].push(element.split('.')[0]);
            }
        })
    });
    for(let key in result)
    {
        console.log(`0x${key},/*${result[key]}*/`);
    }
})