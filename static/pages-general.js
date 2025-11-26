const modalBg = document.getElementById("modal-bg");
const modalGuide = document.getElementById("modal-guide");
const iframe = document.getElementById("iframe");
function showModalGuide(url) {
  modalBg.classList.remove("hidden");
  modalGuide.classList.remove("hidden");
  setTimeout(
    () => {
      modalBg.classList.remove("modal-hidden");
      modalGuide.classList.remove("modal-hidden");
    },
    20
  );
  iframe.src = url;
}
function hideModal() {
  modalBg.classList.add("modal-hidden");
  Array.from(document.getElementsByClassName("modal")).map((x) => x.classList.add("modal-hidden"));
  setTimeout(
    () => {
      modalBg.classList.add("hidden");
      modalGuide.classList.add("hidden");
    },
    400
  );
}
function followLink(link) {
  window.open(link + window.location.search, "_self");
}
function joinGame(gameId) {
  const params = new URLSearchParams(window.location.search);
  params.set("gameId", gameId);
  const newUrl = "/joinGame?" + params.toString();
  window.open(newUrl, "_self");
}
