window.onload = function () {
  document.getElementById("btn").onclick = () => {
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "set", true);
    xhr.send(document.getElementById("text-input").value);
  };
};
