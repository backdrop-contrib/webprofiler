(function ($) {
  "use strict";

  /**
   * Add buttons to messages to allow users to dismiss them.
   */
  Backdrop.behaviors.webprofiler = {
    attach: function (context, settings) {
      var data = settings.webprofiler;
      // Color scale
      var functionColors = d3.scaleLinear()
        .domain([0, 100])
        .range(['#f4e5ec', '#b63c71']);

      var queryColors = d3.scaleLinear()
        .domain([0, 100])
        .range(['#d4f6f4', '#005b56']);

      var textSize = d3.scaleLinear()
        .domain([0, 100])
        .range([1, 3]);

      // Generate the style props for a node and its label
      var nodeStyle = function(node) {
        var ratio = node.time / data.total_time * 100;
        if (node.type === 'function') {
          return 'fill: ' + functionColors(ratio) + ';';
        }
        if (node.type === 'query') {
          return 'fill: ' + queryColors(ratio) + ';';
        }
        return '';
      };

      var nodeLabelStyle = function(node) {
        var ratio = node.time / data.total_time * 100;
        if (node.type === 'group') {
          return 'font-size: 3em;font-weight:bold;';
        }
        return 'font-size: ' + textSize(ratio) + 'em;';
      };

      var nodeLabel = function(node) {
        if (node.type === 'group') {
          return '';//node.name;
        }
        if (node.type === 'function') {
          var word = node.count === 1 ? 'call' : 'calls';
        }
        if (node.type === 'query') {
          var word = node.count === 1 ? 'query' : 'queries';
        }
        return node.name + "\n" + node.count + ' ' + word + ', ' + formatTime(node.time, 0) + ' µs';
      };

      // Create a new directed graph
      var g = new dagreD3.graphlib.Graph({compound:true}).setGraph({});
      g.graph().rankdir = "TB";
      g.graph().ranksep = 10;
      g.graph().nodesep = 10;

      for (var i = 0, len = data.nodes.length; i < len; i++) {
        var node = data.nodes[i];
        g.setNode(node.name, {
          label: nodeLabel(node),
          style: nodeStyle(node),
          labelStyle: nodeLabelStyle(node),
        });
      }

      for (var i = 0, len = data.groups.length; i < len; i++) {
        var group = data.groups[i];
        g.setParent(group.function, group.class);
      }

      for (i = 0, len = data.links.length; i < len; i++) {
        var edge = data.links[i];
        var word = edge.count === 1 ? ' call' : ' calls';
        g.setEdge(edge.from, edge.to, {
          label: edge.count + word,
          curve: d3.curveBasis
        });
      }

      dagreD3.dagre.layout(g);

      var container = d3.select('#callgraph');
      var width = parseInt(container.style('width'), 10),
        height = 700;

      var svg = container.append('svg')
        .attr("viewBox", [0, 0, width, height])
        .attr('class', 'callgraph');

      var group = svg.append("g");

      var render = new dagreD3.render();
      render(group, g);

      svg.call(d3.zoom()
        .extent([[0, 0], [width, height]])
        .on("zoom", zoomed));

      function zoomed({transform}) {
        group.attr("transform", transform);
      }

      function formatTime (num, decimalPlaces) {
        if (decimalPlaces === undefined) {
          decimalPlaces = 2;
        }
        var sep = ',';

        var number = +num;
        var val = number.toFixed(decimalPlaces, 0);
        if (val < 1000) {
          return val;
        }
        var split = val.split(/\./);
        var thousands = split[0];
        var i = thousands.length % 3 || 3;

        thousands = thousands.slice(0, i) + thousands.slice(i).replace(/(\d{3})/g, sep + '$1');
        split[0] = thousands;
        return split.join('.');
      }
    }
  };
})(jQuery);
