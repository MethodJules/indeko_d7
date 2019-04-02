/**
 * Created by hofer on 12.01.2017.
 */
/**
 * @file
 * Javascript for [library name].
 */

(function($) {

    /**
     * Adds library to the global d3 object.
     *
     * @param select
     * @param settings
     *   Array of values passed to d3_draw.
     *   id: required. This will be needed to attach your
     *       visualization to the DOM.
     */
    Drupal.d3.biblio_network = function (select, settings) {
        // Your custom JS.

        var w = 1000;
        var h = 1800;
        var linkDistance=200;
        div = (settings.id) ? settings.id : 'visualization';

        //var dataset = {"nodes":[{"name":0},{"name":1},{"name":2},{"name":3}], "edges":[{"target":0,"source":1},{"target":0,"source":2}]};
        //var dataset = {"nodes":[{"name":"16"},{"name":"18"},{"name":"17"},{"name":"19"}],"edges":[{"source":0,"target":2},{"source":1,"target":3},{"source":1,"target":3}]};
        var dataset = settings.data;




        var colors = d3.scale.category10();


        var svg = d3.select('#' + div).append("svg").attr({"width":w,"height":h})
            .call(d3.behavior.zoom().on("zoom", function () {
                svg.attr("transform", "translate(" + d3.event.translate + ")" + " scale(" + d3.event.scale + ")")
            }))
            .append("g");

        var force = d3.layout.force()
            .nodes(dataset.nodes)
            .links(dataset.edges)
            .size([w,h])
            .linkDistance([linkDistance])
            .charge([-500])
            .theta(0.1)
            .gravity(0.05)
            .start();



        var edges = svg.selectAll("line")
            .data(dataset.edges)
            .enter()
            .append("line")
            .attr("id",function(d,i) {return 'edge'+i})
            //.attr('marker-end','url(#arrowhead)')
            .style("stroke","#000")
            .style("pointer-events", "none")
            .style("stroke-width", "1px");


        var nodes = svg.selectAll("circle")
            .data(dataset.nodes)
            .enter()
            .append("circle")
            .attr({"r":15})
            //.style("fill",function(d,i){return colors(i);})
            .style("fill","green")
            .call(force.drag)
            .on('dblclick', connectedNodes)
            .on('mouseover', handleMouseOver);

        var nodelabels = svg.selectAll(".nodelabel")
            .data(dataset.nodes)
            .enter()
            .append("text")
            .attr({"x":function(d){return d.x;},
                "y":function(d){return d.y;},
                "class":"nodelabel",
                'font-size':12,
                "stroke":"black"})
            .text(function(d){return d.name;});

        var edgepaths = svg.selectAll(".edgepath")
            .data(dataset.edges)
            .enter()
            .append('path')
            .attr({'d': function(d) {return 'M '+d.source.x+' '+d.source.y+' L '+ d.target.x +' '+d.target.y},
                'class':'edgepath',
                'fill-opacity':0,
                'stroke-opacity':0,
                'fill':'blue',
                'stroke':'red',
                'id':function(d,i) {return 'edgepath'+i}})
            .style("pointer-events", "none");

        var edgelabels = svg.selectAll(".edgelabel")
            .data(dataset.edges)
            .enter()
            .append('text')
            .style("pointer-events", "none")
            .attr({'class':'edgelabel',
                'id':function(d,i){return 'edgelabel'+i},
                'dx':80,
                'dy':0,
                'font-size':10,
                'fill':'#aaa'});



        svg.append('defs').append('marker')
            .attr({'id':'arrowhead',
                'viewBox':'-0 -5 10 10',
                'refX':25,
                'refY':0,
                //'markerUnits':'strokeWidth',
                'orient':'auto',
                'markerWidth':10,
                'markerHeight':10,
                'xoverflow':'visible'})
            .append('svg:path')
            .attr('d', 'M 0,-5 L 10 ,0 L 0,5')
            .attr('fill', '#ccc')
            .attr('stroke','#ccc');


        force.on("tick", function(){

            edges.attr({"x1": function(d){return d.source.x;},
                "y1": function(d){return d.source.y;},
                "x2": function(d){return d.target.x;},
                "y2": function(d){return d.target.y;}
            });

            nodes.attr({"cx":function(d){return d.x;},
                "cy":function(d){return d.y;}
            });

            nodelabels.attr("x", function(d) { return d.x; })
                .attr("y", function(d) { return d.y; });

            edgepaths.attr('d', function(d) { var path='M '+d.source.x+' '+d.source.y+' L '+ d.target.x +' '+d.target.y;
                //console.log(d)
                return path});

            edgelabels.attr('transform',function(d,i){
                if (d.target.x<d.source.x){
                    bbox = this.getBBox();
                    rx = bbox.x+bbox.width/2;
                    ry = bbox.y+bbox.height/2;
                    return 'rotate(180 '+rx+' '+ry+')';
                }
                else {
                    return 'rotate(0)';
                }
            });
        });

        //------------------------------------------------

        //Toggle stores whether the highlighting is on
        var toggle = 0;
        //Create an array logging what is connected to what
        var linkedByIndex = {};
        for (i = 0; i < dataset.nodes.length; i++) {
            linkedByIndex[i + "," + i] = 1;
        };
        dataset.edges.forEach(function (d) {
            linkedByIndex[d.source.index + "," + d.target.index] = 1;
        });
        //This function looks up whether a pair are neighbours
        function neighboring(a, b) {
            return linkedByIndex[a.index + "," + b.index];
        }
        function connectedNodes() {
            if (toggle == 0) {
                //Reduce the opacity of all but the neighbouring nodes
                d = d3.select(this).node().__data__;
                nodes.style("opacity", function (o) {
                    return neighboring(d, o) | neighboring(o, d) ? 1 : 0.1;
                });
                edges.style("opacity", function (o) {
                    return d.index==o.source.index | d.index==o.target.index ? 1 : 0.1;
                });
                //Reduce the op
                toggle = 1;
            } else {
                //Put them back to opacity=1
                nodes.style("opacity", 1);
                edges.style("opacity", 1);
                toggle = 0;
            }
        }

        //------------------------------------------------
        function hrefNodes(){
            window.location.href = d.url;
        }
        //------------------------------------------------

        //------------------------------------------------
    }

    function hello_d3() {
        alert('Hallo');
    }

    function handleMouseOver(d,i) {
        //alert(i);
        // $.ajax({
        //     url: 'netzwerk/calc/100',
        //     success: function (data) {
        //         $('#block-network-network').html('<h2>Juhuuh</h2>' + data);
        //     }
        // });
    }

})(jQuery);
