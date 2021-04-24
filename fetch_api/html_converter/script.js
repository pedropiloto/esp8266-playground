const fs = require('fs');
const minify = require('html-minifier').minify;

const myArgs = process.argv.slice(2);


fs.readFile(myArgs[0], 'utf8', function(err, data) {
    if (err) throw err;

    //html-minifier --collapse-whitespace --remove-comments --remove-optional-tags --remove-redundant-attributes --remove-script-type-attributes --remove-tag-whitespace --use-short-doctype --minify-css true --minify-js true
    let result = minify(data,
        {
        collapseWhitespace: true,
        removeComments: true,
        removeRedundantAttributes: true,
        removeTagWhitespace: true,
        minifyCSS: true,
        minifyJS: true
        }
        )
    html = JSON.stringify(result);

    fs.writeFile(myArgs[1], `const String main_page_html = ${html};`, function(err) {
        if(err) {
            return console.log(err);
        }
        console.log("The file was saved!");
    }); 

});