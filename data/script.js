window.onload = function () {
  const symbols =
    " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`{|}~ЁАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ№";
  function post() {
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "set", true);
    const text =
      document
        .getElementById("text-input")
        .value.toLocaleUpperCase()
        .split("")
        .filter((x) => symbols.indexOf(x) >= 0)
        .join("") || "";
    let buffer = new ArrayBuffer(text.length);
    let view = new Uint8Array(buffer);
    for (var i = 0; i < text.length; i++) {
      view[i] = symbols.indexOf(text.charAt(i));
    }
    console.log(view, buffer);
    xhr.send(buffer);
  }
  document.getElementById("btn").onclick = post;
  document
    .getElementById("text-input")
    .addEventListener("keyup", function (event) {
      if (event.keyCode === 13) post();
    });
};
