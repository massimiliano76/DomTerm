/* Used for saved (snapshots of) DomTerm sessions. */
function loadHandler(event) {
    var dt = new DomTerm("domterm");
    window.domterm1 = dt;
    var topNode = document.getElementById("domterm");
    if (topNode.getAttribute("class") == DomTerm._savedSessionClassNoScript)
        topNode.setAttribute("class", DomTerm._savedSessionClass);
    dt.setWindowSize = function(numRows, numColumns,
                                availHeight, availWidth) {
    };
    dt.initial = document.getElementById(dt.makeId("main"));
    dt._restoreLineTables(topNode, 0);
    dt._initializeDomTerm(topNode);
    dt._breakAllLines();

    topNode.addEventListener("click",
                             function(e) {
                                 var target = e.target;
                                 if (target instanceof Element
                                     && target.nodeName == "SPAN"
                                     && target.getAttribute("class") == "term-style"
                                     && target.getAttribute("std") == "hider") {
                                     dt._showHideHandler(e);
                                     e.preventDefault();
                                 }
                             },
                             false);
    
};
window.addEventListener("load", loadHandler, false);
