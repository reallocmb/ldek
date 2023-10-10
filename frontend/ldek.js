let word_pointer = 0;
let word_line = 0;
let word_pointer_old = 0;
let word_line_old = 0;
let mode = "NORMAL";

let pointer_update = function()
{
    document.querySelector('#dek-text')
        .children[word_pointer_old].children[word_line_old]
        .style.background = 'gray';

    document.querySelector('#dek-text')
        .children[word_pointer].children[word_line]
        .style.background = 'violet';
}

let mode_change = function()
{
    if (mode === "NORMAL")
    {
        mode = "INSERT";
    }
    else
    {
        mode = "NORMAL";
    }
    document.querySelector('#mode-display').textContent = mode;
}

let word_insert = function (index) {
    document.querySelector('#dek-text').children[word_pointer].children[word_line].focus();
}

let dek_object_append = function (sword = "", dword = "")
{
    const helper = document.createElement('div');
    helper.innerHTML += `
        <div class="dek-object">
        <p class="word-object" contenteditable="true">${sword}</p>
        <p class="word-object" contenteditable="true">${dword}</p>
        </div>`
    const dek_text = document.querySelector('#dek-text');
    dek_text.insertBefore(helper.children[0], dek_text.children[word_pointer + 1]);
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

let dek_object_delete = function() {
}

let word_pointer_linestart = function() {
    const row_current = dek_object_row_get(word_pointer);

    let i;
    for (i = word_pointer - 1; i >= 0; i--) {
        if (row_current > dek_object_row_get(i)) {
            word_pointer = i + 1;
            return;
        }
    }

    word_pointer = 0;
}

let word_pointer_linedown = function() {
    const row_current = dek_object_row_get(word_pointer);
    const length = document.querySelector('#dek-text').childElementCount;

    let i;
    for (i = word_pointer + 1; i < length; i++) {
        if (row_current < dek_object_row_get(i)) {
            word_pointer = i;
            break;
        }
    }
}

let word_pointer_lineup = function() {
    const row_current = dek_object_row_get(word_pointer);

    let i;
    for (i = word_pointer - 1; i >= 0; i--) {
        if (row_current > dek_object_row_get(i)) {
            word_pointer = i;
            word_pointer_linestart();
            break;
        }
    }
}

let word_pointer_lineend = function() {
    const row_current = dek_object_row_get(word_pointer);
    const length = document.querySelector('#dek-text').childElementCount;

    let i;
    for (i = word_pointer + 1; i < length; i++) {
        if (row_current < dek_object_row_get(i)) {
            word_pointer = i - 1;
            return;
        }
    }
    word_pointer = length - 1;
}

document.activeElement.addEventListener('input', function (e) {
    word_pointer_old = word_pointer;
    word_line_old = word_line;

    if (e.data === ' ')
    {
        if (word_line)
        {
            word_pointer++;
        }
        else
        {
            console.log('append object');
            dek_object_append();
            word_pointer++;
        }
        word_insert();
    }
    pointer_update();
});

window.addEventListener('keydown', function (e) {
    word_pointer_old = word_pointer;
    word_line_old = word_line;

    if (e.key === 'Tab')
    {
        e.preventDefault();
        if (mode === "NORMAL")
        {
            word_insert(word_pointer);
        }
        else
        {
            document.activeElement.blur();
        }
        mode_change();
    }

    if (mode === "NORMAL")
    {
        if (e.key === 's')
            word_pointer--;
        if (e.key === 'n')
            word_line++;
        if (e.key === 'r')
            word_line--;
        if (e.key === 't')
            word_pointer++;
        if (e.key === 'S')
            word_pointer_linestart();
        if (e.key === 'N')
            word_pointer_linedown();
        if (e.key === 'R')
            word_pointer_lineup();
        if (e.key === 'T')
            word_pointer_lineend();
        if (e.key === 'd')
            dek_object_delete();



        if (word_pointer < 0)
            word_pointer = 0;
        if (word_pointer >= document.querySelector('#dek-text').childElementCount)
            word_pointer = document.querySelector('#dek-text').childElementCount - 1;
        if (word_line < 0)
            word_line = 0;
        if (word_line > 1)
            word_line = 1;
    }

    pointer_update();
    console.log('word_pointer: ' + word_pointer + 'word_line: ' + word_line);
});

let dek_new = function()
{
    document.querySelector('#dek-text').innerHTML = "";
    document.querySelector('#file-name').textContent = prompt();
    dek_object_append();
    pointer_update();
}

let dek_save = function()
{
    let obj_array = [];
    let childs_length = document.querySelector('#dek-text').childElementCount;
    let childs = document.querySelector('#dek-text').children;

    let i;
    for (i = 0; i < childs_length; i++)
    {
        obj_array.push({ sword: childs[i].children[0].textContent, dword: childs[i].children[1].textContent });
    }

    let obj_str = JSON.stringify(obj_array);
    /*
    window.localStorage.setItem(document.querySelector('#file-name').textContent, obj_str);
    */

    const url = '/save';

    const xhr = new XMLHttpRequest();
    xhr.open('POST', url, true);
    xhr.setRequestHeader('Content-Type', 'application/json;charset=UTF-8');

    xhr.onreadystatechange = function () {
        if (xhr.readyState === XMLHttpRequest.DONE) {
            if (xhr.status === 204) {
                console.log('POST-Request erfolgreich');
                console.log('Antwort vom Server:', xhr.responseText);
            } else {
                console.error('Fehler beim POST-Request');
            }
        }
    };

    xhr.send(document.querySelector('#file-name').textContent + ";" + obj_str);
}

let dek_open = function()
{
    const url = '/open';

    const xhr = new XMLHttpRequest();
    xhr.open('POST', url, true);

    xhr.onreadystatechange = function () {
        if (xhr.readyState === XMLHttpRequest.DONE) {
            if (xhr.status === 302) {
                console.log('POST-Request erfolgreich');
                document.querySelector('#dek-text').innerHTML = '';
                let dek_list = document.querySelector('#dek-text');
                let container = document.createElement('div');
                container.className = "container";
                dek_list.appendChild(container);

                let array = xhr.responseText.split(',');
                if (array == "")
                {
                    let p = document.createElement('p');
                    p.textContent = 'Empty';
                    container.appendChild(p);
                    return;
                }
                array.forEach((item, index) => {
                    let container_item = document.createElement('div');
                    container_item.className = "container-item";
                    let p = document.createElement('p');
                    p.textContent = item;
                    let close = document.createElement('button');
                    close.addEventListener('click', (e) => {
                        dek_delete(item);
                    });
                    container_item.append(p, close);

                    p.addEventListener('click', (e) => { 
                        let file_name_label = document.querySelector('#file-name');
                        file_name_label.textContent = e.target.textContent;

                        const url = '/load';

                        const xhr = new XMLHttpRequest();
                        xhr.open('POST', url, true);

                        xhr.onreadystatechange = function () {
                            if (xhr.readyState === XMLHttpRequest.DONE) {
                                if (xhr.status === 302) {
                                    console.log('POST-Request erfolgreich');
                                    console.log('Antwort vom Server:', xhr.responseText);

                                    let dek_obj_array = JSON.parse(xhr.responseText);
                                    document.querySelector('#dek-text').innerHTML = "";
                                    word_pointer = 0;
                                    dek_obj_array.forEach((item) => {
                                        dek_object_append(item.sword, item.dword);
                                        word_pointer++;
                                    });
                                    word_pointer = 0;
                                    word_line = 0;
                                    pointer_update();
                                } else {
                                    console.error('Fehler beim POST-Request');
                                }
                            }
                        };

                        xhr.send(file_name_label.textContent);

                    });
                    container.append(container_item);
                });
                console.log('Antwort vom Server:', xhr.responseText);
            } else {
                console.error('Fehler beim POST-Request');
            }
        }
    };
    xhr.send();

    return;
}

let dek_delete = function(name) {
    const url = '/delete';

    const xhr = new XMLHttpRequest();
    xhr.open('POST', url, true);

    xhr.onreadystatechange = function () {
        if (xhr.readyState === XMLHttpRequest.DONE) {
            if (xhr.status === 200) {
                console.log('POST-Request erfolgreich');
                console.log('Antwort vom Server:', xhr.responseText);
                dek_open();
            } else {
                console.error('Fehler beim POST-Request');
            }
        }
    };

    xhr.send(name);
}


window.onload = function()
{
}

