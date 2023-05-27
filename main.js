'use strict'

// Mode
// 0 -> Normal
// 1 -> Insert
let mode = 0;
let box_index = 0;
let word_index = 0;

let file_name = "";

let dek_text = document.querySelector('#dek-text');

let update = function () {
    //dek_text.children[box_index].children[word_index].classList.add('word-pointed');
    if (mode) {
        dek_text.children[box_index].children[word_index].style.background = "violet";
    } else {
        dek_text.children[box_index].children[word_index].style.background = "gray";
    }
}
let box_append = function () {
    const helper = document.createElement("div");
    helper.innerHTML += `
        <div class="dek-object">
        <p class="source-word word"></p>
        <p class="destination-word word"></p>
        </div>`
    dek_text.insertBefore(helper.children[0], dek_text.children[box_index + 1]);
}

let dek_append = function(sword, dword) {
    document.querySelector('#dek-text').innerHTML += `
        <div class="dek-object">
        <p class="source-word word">${sword}</p>
        <p class="destination-word word">${dword}</p>
        </div>`
    box_index++;
}

let clear = function() {
    dek_text = document.querySelector('#dek-text');
    dek_text.children[box_index].children[word_index].classList.remove('word-pointed');
    dek_text.children[box_index].children[word_index].style.background = "";
}

let dek_object_row_get = function(index) {
    let row = 0;
    let dek_text = document.querySelector("#dek-text");
    let width = dek_text.offsetWidth;

    let children = dek_text.children;

    let curr_width = 0;
    let i;
    for (i = 0; i <= index; i++) {
        curr_width += children[i].offsetWidth;
        if (width < curr_width ) {
            row++;
            curr_width = children[i].offsetWidth;
        }
    }

    return row;
}

document.addEventListener('keydown', (e) => {
    clear();
    if (!mode) {
        if (e.key === "i") {
            mode = 1;
        }
        if (e.key === "t") {
            box_index++;
        }
        if (e.key === "s") {
            box_index--;
        }
        if (e.key === "n") {
            word_index++;
        }
        if (e.key === "r") {
            word_index--;
        }
        if (e.key == "d") {
            dek_text.children[box_index].remove();
        }
    } else {
        if (e.key == " ") {
            if (word_index) {
            } else {
                box_append();
            }
            box_index++;
        }
        if (e.key == "_") {
            dek_text.children[box_index].children[word_index].textContent = dek_text.children[box_index].children[word_index].textContent + " ";
        }
        if (e.key === "Escape") {
            mode = 0;
        }
        console.log(e.key);
        if (e.key == "Backspace") {
            dek_text.children[box_index].children[word_index].textContent = dek_text.children[box_index].children[word_index].textContent.slice(0, -1);
        }
        if (!(e.key == "Shift" || e.key == "Control" || e.key == "AltGraph" || e.key == "Escape" || e.key == "_" || e.key == "Backspace")) {
            dek_text.children[box_index].children[word_index].textContent = dek_text.children[box_index].children[word_index].textContent + e.key;
        }
    }
    update();
});

let dek_save = function() {
    let children = dek_text.children;
    let obj;
    // Get all "dek-object" elements
    const dekObjects = Array.from(document.getElementsByClassName('dek-object'));

    // Create an array to store the word pairs
    const wordPairs = [];

    // Iterate over each "dek-object" element
    dekObjects.forEach(dekObject => {
        // Find the source and destination words within the current "dek-object"
        const sourceWord = dekObject.querySelector('.source-word').textContent.trim();
        const destinationWord = dekObject.querySelector('.destination-word').textContent.trim();

        // Create a word pair object
        const wordPair = {
            sourceWord: sourceWord,
            destinationWord: destinationWord
        };

        // Add the word pair object to the array
        wordPairs.push(wordPair);
    });

    // Convert the word pairs array to JSON format
    const json = JSON.stringify(wordPairs);

    let file_name_label = document.querySelector('#file-name');
    window.localStorage.setItem(file_name_label.textContent, json);

}

let dek_new = function() {
    dek_text.innerHTML = "";
    box_index = 0;
    file_name = prompt();
    let file_name_label = document.querySelector('#file-name');

    file_name_label.textContent = file_name;

    box_append();
    update();
}

let dek_open = function() {
    document.body.innerHTML += `
        <div id="dek-list">
        </div>`
    let dek_list = document.querySelector('#dek-list');
    let array = Object.keys(window.localStorage);
    array.forEach((item, index) => {
        let p = document.createElement('p');
        p.textContent = item;
        p.addEventListener('click', (e) => { 
            let file_name_label = document.querySelector('#file-name');
            file_name_label.textContent = e.target.textContent;

            let dek_text_array = JSON.parse(localStorage.getItem(e.target.textContent) || "[]");
            dek_text_array.forEach((item) => {
                dek_append(item["sourceWord"], item["destinationWord"]);
            });
            box_index = 0;

            dek_list.parentNode.removeChild(dek_list);
        });
    
        dek_list.append(p);
    });
}

box_append();
update();
