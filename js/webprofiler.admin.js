/**
 * Adds an event listener for searching the function list.
 */
Backdrop.behaviors.functionFilter = {
  attach: function (context, settings) {
    var table = document.getElementById('function-table');
    var search = document.getElementById('function-search');
    if (typeof(table) !== 'undefined' && typeof(search) !== 'undefined') {
      var rows = table.children[1].children;
      search.addEventListener('input', function (event) {
        var text = event.target.value;
        for(var i = 0; i < rows.length; i++) {
          if (rows[i].dataset.function.includes(text)) {
            if (rows[i].classList.contains('hidden')) {
              rows[i].classList.remove('hidden');
            }
          }
          else {
            if (!rows[i].classList.contains('hidden')) {
              rows[i].classList.add('hidden');
            }
          }
        }

      });
    }
  }
};
