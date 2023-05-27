'use strict'

let mode_current = "NORMAL";
let word_pointer = { word: 0, lang: 0 };

let file_name;

let word_pointer_check = function() {
    if (word_pointer.word < 0)
        word_pointer.word = 0;
    if (word_pointer.lang < 0)
        word_pointer.lang = 0;
    if (word_pointer.lang > 1)
        word_pointer.lang = 1;
    let dek_text = document.querySelector('#dek-text');
    if (word_pointer.word >= dek_text.childElementCount)
        word_pointer.word = dek_text.childElementCount - 1;
}

let word_pointer_update_mode_normal = function() {
    word_pointer_check();
    document.querySelector('#dek-text')
        .children[word_pointer.word].children[word_pointer.lang]
        .style.background = "gray";
}

let word_pointer_update_mode_insert = function() {
    word_pointer_check();
    document.querySelector('#dek-text')
        .children[word_pointer.word].children[word_pointer.lang]
        .style.background = "violet";
}

let mode_switch = function() {
    if (mode_current == "NORMAL")
        mode_current = "INSERT";
    else
        mode_current = "NORMAL";

    let mode = document.querySelector('#mode-display');
    mode.textContent = mode_current;
}

let dek_object_append = function(word_position = 0, sword = "", dword = "") {
    const helper = document.createElement('div');
    helper.innerHTML += `
        <div class="dek-object">
        <p class="source-word word">${sword}</p>
        <p class="destination-word word">${dword}</p>
        </div>`
    const dek_text = document.querySelector('#dek-text');
    dek_text.insertBefore(helper.children[0], dek_text.children[word_position + 1]);
}

let dek_new = function(e) {
    dek_save(e);
    document.querySelector('#dek-text').innerHTML = "";
    dek_object_append();
    word_pointer_update_mode_normal();

    // file name
    file_name = prompt();
    let file_name_label = document.querySelector('#file-name');

    file_name_label.textContent = file_name;

    e.preventDefault();
}

let dek_save = function(e) {
    let obj_array = [];
    let childs_length = document.querySelector('#dek-text').childElementCount;
    let childs = document.querySelector('#dek-text').children;

    let i;
    for (i = 0; i < childs_length; i++) {
        obj_array.push({ sword: childs[i].children[0].textContent, dword: childs[i].children[1].textContent });
    };
    let obj_str = JSON.stringify(obj_array);
    window.localStorage.setItem(document.querySelector('#file-name').textContent, obj_str);
    e.preventDefault();
}

let dek_delete = function(e) {
    localStorage.removeItem(document.querySelector('#file-name').textContent);
    document.querySelector('#file-name').textContent = "";
    document.querySelector('#dek-text').innerHTML = "";
    e.preventDefault();
}

let dek_open = function(e) {
    dek_save(e);
    document.querySelector('#dek-text').innerHTML = '';
    let dek_list = document.querySelector('#dek-text');
    let array = Object.keys(window.localStorage);
    array.forEach((item, index) => {
        let p = document.createElement('p');
        p.textContent = item;
        p.addEventListener('click', (e) => { 
            let file_name_label = document.querySelector('#file-name');
            file_name_label.textContent = e.target.textContent;

            let dek_obj_array = JSON.parse(localStorage.getItem(e.target.textContent));
            document.querySelector('#dek-text').innerHTML = "";
            dek_obj_array.forEach((item) => {
                dek_object_append(word_pointer.word, item.sword, item.dword);
                word_pointer.word++;
            });
            word_pointer.word = 0;
            word_pointer.lang = 0;
            word_pointer_update_mode_normal();

            // dek_list.parentNode.removeChild(dek_list);
        });
    
        dek_list.append(p);
    });
    e.preventDefault();
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

let word_pointer_linestart = function() {
    const row_current = dek_object_row_get(word_pointer.word);

    let i;
    for (i = word_pointer.word - 1; i >= 0; i--) {
        if (row_current > dek_object_row_get(i)) {
            word_pointer.word = i + 1;
            return;
        }
    }

    word_pointer.word = 0;
}

let word_pointer_lineend = function() {
    const row_current = dek_object_row_get(word_pointer.word);
    const length = document.querySelector('#dek-text').childElementCount;

    let i;
    for (i = word_pointer.word + 1; i < length; i++) {
        if (row_current < dek_object_row_get(i)) {
            word_pointer.word = i - 1;
            return;
        }
    }
    word_pointer.word = length - 1;
}

let word_pointer_linedown = function() {
    const row_current = dek_object_row_get(word_pointer.word);
    const length = document.querySelector('#dek-text').childElementCount;

    let i;
    for (i = word_pointer.word + 1; i < length; i++) {
        if (row_current < dek_object_row_get(i)) {
            word_pointer.word = i;
            break;
        }
    }
}

let word_pointer_lineup = function() {
    const row_current = dek_object_row_get(word_pointer.word);

    let i;
    for (i = word_pointer.word - 1; i >= 0; i--) {
        if (row_current > dek_object_row_get(i)) {
            word_pointer.word = i;
            word_pointer_linestart();
            break;
        }
    }
}

let dek_import = function() {
    let text = prompt();
    text = text.replace(/\n/g, ' ');
    let array = text.split(' ');
    array.forEach((item) => {
        if (item != "") {
            dek_object_append(word_pointer.word, item);
            word_pointer.word++;
        }
    });
}

// key input handler
const super_key = ' ';
let super_key_was_pressed = 0;
window.addEventListener('keydown', function (e) {
    console.log(e.key);
    e.preventDefault();
    let dek_text = document.querySelector('#dek-text');
    if (dek_text.childElementCount > 0)
        dek_text.children[word_pointer.word].children[word_pointer.lang].style.background = '';

    // new file
    if (super_key_was_pressed) {
        super_key_was_pressed = 0;
        switch (e.key) {
            case 'n': dek_new(e); break;
            case 'o': dek_open(e); break;
            case 'd': dek_delete(e); break;
            case 'w': dek_save(e); break;
            case 'i': dek_import(); break;
        }
        return;
    }
    
    switch (e.key) {
        case 'Tab':
            mode_switch();
            e.preventDefault();
            break;
    }

    if (mode_current == "NORMAL") {
        switch (e.key) {
            case super_key: super_key_was_pressed = 1; return; break;
            case 's': word_pointer.word--; break;
            case 'n': word_pointer.lang++; break;
            case 'r': word_pointer.lang--; break;
            case 't': word_pointer.word++; break;
            case 'g': word_pointer.word = 0; break;
            case 'G': word_pointer.word = document.querySelector('#dek-text').childElementCount - 1; break;

            case 'S': word_pointer_linestart(); break;
            case 'N': word_pointer_linedown(); break;
            case 'R': word_pointer_lineup(); break;
            case 'T': word_pointer_lineend(); break;

            case 'd': document.querySelector('#dek-text').children[word_pointer.word].remove(); break;
        }
        word_pointer_update_mode_normal();
    } else {
        let dek_text_word = document.querySelector('#dek-text').children[word_pointer.word].children[word_pointer.lang];
        switch (e.key) {
            case ' ':
                if (word_pointer.lang) {
                    word_pointer.word++;
                } else {
                    dek_object_append(word_pointer.word);
                    word_pointer.word++;
                }
                break;
            case '_':
                dek_text_word.textContent += ' ';
                break;
            case 'Enter': break;
            case 'Control': break;
            case 'Shift': break;
            case 'Escape': break;
            case 'Alt': break;
            case 'AltGraph': break;
            case 'Tab': break;
            case 'Backspace':
                dek_text_word.textContent = dek_text_word.textContent.slice(0, -1);
                break;
            default:
                dek_text_word.textContent = dek_text_word.textContent + e.key;
                break;
        }
        e.preventDefault();
        word_pointer_update_mode_insert();
    }
});
