"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const fs = require("fs");
const dict = new Map();
function addWord(word) {
    const val = dict.get(word);
    if (val)
        dict.set(word, val + 1);
    else
        dict.set(word, 1);
}
function isLetter(code) {
    return code > 64 && code < 91
        || code > 96 && code < 123;
}
function writeFile(filePath) {
    const fstream = fs.createWriteStream(filePath);
    const out = Array.from(dict.entries()).sort((kv1, kv2) => {
        if (kv1[1] < kv2[1]) {
            return 1;
        }
        else if (kv1[1] == kv2[1]) {
            return kv1[0] > kv2[0] ? 1 : -1;
        }
        else {
            return -1;
        }
    });
    try {
        for (let i = 0; i < out.length; i++) {
            fstream.write(out[i][1] + " " + out[i][0] + "\n");
        }
    }
    finally {
        fstream.close();
    }
}
function main(args) {
    if (args.length != 4)
        return;
    const start = (new Date()).getTime();
    let word = "";
    const stream = fs.createReadStream(args[2]);
    stream.on('readable', () => {
        let data;
        while (null !== (data = stream.read())) {
            for (let i = 0; i < data.length; i++) {
                const ch = data[i];
                if (isLetter(ch)) {
                    word += String.fromCharCode(ch);
                    ;
                    continue;
                }
                if (word.length == 0)
                    continue;
                addWord(word.toLowerCase());
                word = "";
            }
        }
    });
    stream.on('end', () => {
        if (word.length > 0)
            addWord(word.toLowerCase());
        writeFile(args[3]);
        console.log(`Time ${(new Date()).getTime() - start} ms`);
    });
}
main(process.argv);
//# sourceMappingURL=app.js.map