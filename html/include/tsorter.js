/*!
 * tsorter 2.0.0 - Copyright 2015 Terrill Dent, http://terrill.ca
 * JavaScript HTML Table Sorter
 * Released under MIT license, http://terrill.ca/sorting/tsorter/LICENSE
 */
var tsorter = (function()
{
    'use strict';

    var sorterPrototype,
        addEvent,
        removeEvent,
        hasEventListener = !!document.addEventListener;

    if( !Object.create ){
        // Define Missing Function
        Object.create = function( prototype ) {
            var Obj = function(){return undefined;};
            Obj.prototype = prototype;
            return new Obj();
        };
    }

    // Cross Browser event binding
    addEvent = function( element, eventName, callback ) {
        if( hasEventListener ) {
            element.addEventListener(eventName, callback, false );
        } else {
            element.attachEvent( 'on' + eventName, callback);
        }
    };

    // Cross Browser event removal
    removeEvent = function( element, eventName, callback ) {
        if( hasEventListener ) {
            element.removeEventListener(eventName, callback, false );
        } else {
            element.detachEvent( 'on' + eventName, callback);
        }
    };

    sorterPrototype = {

        getCell: function(row)
        {
            var that = this;
            return that.trs[row].cells[that.column];
        },

        /* SORT
         * Sorts a particular column. If it has been sorted then call reverse
         * if not, then use quicksort to get it sorted.
         * Sets the arrow direction in the headers.
         * @param oTH - the table header cell (<th>) object that is clicked
         */
        sort: function( e )
        {
            var that = this,
                th = e.target;

            // TODO: make sure target 'th' is not a child element of a <th>
            //  We can't use currentTarget because of backwards browser support
            //  IE6,7,8 don't have it.

            // set the data retrieval function for this column
            that.column = th.cellIndex;
            that.get = that.getAccessor( th.getAttribute('data-tsorter') );

            if( that.prevCol === that.column )
            {
                // if already sorted, reverse
                th.className = th.className !== 'ascend' ? 'ascend' : 'descend';
                that.reverseTable();
            }
            else
            {
                // not sorted - call quicksort
                th.className = 'ascend';
                if( that.prevCol !== -1 && that.ths[that.prevCol].className !== 'exc_cell'){
                    that.ths[that.prevCol].className = '';
                }
                that.quicksort(0, that.trs.length);
            }
            that.prevCol = that.column;
        },

        /*
         * Choose Data Accessor Function
         * @param: the html structure type (from the data-type attribute)
         */
        getAccessor: function(sortType)
        {
            var that = this,
                accessors = that.accessors;

            if( accessors && accessors[ sortType ] ){
                return accessors[ sortType ];
            }

            switch( sortType )
            {
                case "link":
                    return function(row){
                        return that.getCell(row).firstChild.firstChild.nodeValue;
                    };
                case "input":
                    return function(row){
                        return that.getCell(row).firstChild.value;
                    };
                case "numeric":
                    return function(row){
                        return parseFloat( that.getCell(row).firstChild.nodeValue, 10 );
                    };
                default: /* Plain Text */
                    return function(row){
                        return that.getCell(row).firstChild.nodeValue;
                    };
            }
        },

        /* Exchange
         * A complicated way of exchanging two rows in a table.
         * Exchanges rows at index i and j
         */
        exchange: function(i, j)
        {
            var that = this,
                tbody = that.tbody,
                trs = that.trs,
                tmpNode;

            if( i === j+1 ) {
                tbody.insertBefore(trs[i], trs[j]);
            } else if( j === i+1 ) {
                tbody.insertBefore(trs[j], trs[i]);
            } else {
                tmpNode = tbody.replaceChild(trs[i], trs[j]);
                if( !trs[i] ) {
                    tbody.appendChild(tmpNode);
                } else {
                    tbody.insertBefore(tmpNode, trs[i]);
                }
            }
        },

        /*
         * REVERSE TABLE
         * Reverses a table ordering
         */
        reverseTable: function()
        {
            var that = this,
                i;

            for( i = 1; i < that.trs.length; i++ ) {
                that.tbody.insertBefore( that.trs[i], that.trs[0] );
            }
        },

        /*
         * QUICKSORT
         * @param: lo - the low index of the array to sort
         * @param: hi - the high index of the array to sort
         */
        quicksort: function(lo, hi)
        {
            var i, j, pivot,
                that = this;

            if( hi <= lo+1 ){ return; }

            if( (hi - lo) === 2 ) {
                if(that.get(hi-1) > that.get(lo)) {
                    that.exchange(hi-1, lo);
                }
                return;
            }

            i = lo + 1;
            j = hi - 1;

            if( that.get(lo) > that.get( i) ){ that.exchange( i, lo); }
            if( that.get( j) > that.get(lo) ){ that.exchange(lo,  j); }
            if( that.get(lo) > that.get( i) ){ that.exchange( i, lo); }

            pivot = that.get(lo);

            while(true) {
                j--;
                while(pivot > that.get(j)){ j--; }
                i++;
                while(that.get(i) > pivot){ i++; }
                if(j <= i){ break; }
                that.exchange(i, j);
            }
            that.exchange(lo, j);

            if((j-lo) < (hi-j)) {
                that.quicksort(lo, j);
                that.quicksort(j+1, hi);
            } else {
                that.quicksort(j+1, hi);
                that.quicksort(lo, j);
            }
        },

        init: function( table, initialSortedColumn, customDataAccessors ){
            var that = this,
                i;

            if( typeof table === 'string' ){
                table = document.getElementById(table);
            }

            that.table = table;
            that.ths   = table.getElementsByTagName("th");
            that.tbody = table.tBodies[0];
            that.trs   = that.tbody.getElementsByTagName("tr");
            that.prevCol = ( initialSortedColumn && initialSortedColumn > 0 ) ? initialSortedColumn : -1;
            that.accessors = customDataAccessors;
            that.boundSort = that.sort.bind( that );

            for( i = 0; i < that.ths.length; i++ ) {
                addEvent( that.ths[i], 'click', that.boundSort );
            }
        },

        destroy: function(){
            var that = this,
                i;

            if( that.ths ){
                for( i = 0; i < that.ths.length; i++ ) {
                    removeEvent( that.ths[i], 'click', that.boundSort );
                }
            }
        }
    };

    // Create a new sorter given a table element
    return {
        create: function( table, initialSortedColumn, customDataAccessors )
        {
            var sorter = Object.create( sorterPrototype );
            sorter.init( table, initialSortedColumn, customDataAccessors );
            return sorter;
        }
    };
}());
