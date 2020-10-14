window.onload = function () {
  const symbols =
    " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`{|}~ЁАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ№";
  const input = document.getElementById("text-input");
  function post() {
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "set", true);
    const text =
      input.value
        .toLocaleUpperCase()
        .split("")
        .filter((x) => symbols.indexOf(x) >= 0)
        .join("") || "";
    let buffer = new ArrayBuffer(text.length);
    let view = new Uint8Array(buffer);
    for (var i = 0; i < text.length; i++) {
      view[i] = symbols.indexOf(text.charAt(i));
    }
    // console.log(view, buffer);
    xhr.send(buffer);
    if (
      !Array.from(document.getElementsByClassName("history-item"))
        .map((elem) => elem.innerHTML)
        .includes(text)
    ) {
      var newEntry = document.createElement("p");
      newEntry.innerHTML = text;
      newEntry.classList.add("history-item");
      document.getElementById("history").prepend(newEntry);
    }
  }
  document.getElementById("btn").onclick = post;
  input.addEventListener("keyup", function (event) {
    if (event.keyCode === 13) post();
  });
  input.focus();
  Array.from(document.getElementsByClassName("history-item")).forEach((elem) =>
    elem.addEventListener("click", () => {
      input.value = elem.innerHTML;
      post();
    })
  );
};
