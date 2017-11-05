        /* Regression.JS */
        /* the librar is embedded for your convenience .*/
        /**
         * @license
         *
         * Regression.JS - Regression functions for javascript
         * http://tom-alexander.github.com/regression-js/
         *
         * copyright(c) 2013 Tom Alexander
         * Licensed under the MIT license.
         *
         * @module regression - Least-squares regression functions for JavaScript
         **/
        ! function(a, b) {
            var c;
            return c = "function" == typeof define && define.amd ? define("regression", b) : "undefined" != typeof module ? module.exports = b() : a.regression = b()
        }(this, function() {
            "use strict";

            function a(a, b) {
                var c = a.reduce(function(a, b) {
                        return a + b[1]
                    }, 0),
                    d = c / a.length,
                    e = a.reduce(function(a, b) {
                        var c = b[1] - d;
                        return a + c * c
                    }, 0),
                    f = a.reduce(function(a, c, d) {
                        var e = b[d],
                            f = c[1] - e[1];
                        return a + f * f
                    }, 0);
                return 1 - f / e
            }

            function b(a, b) {
                var c = 0,
                    d = 0,
                    e = 0,
                    f = 0,
                    g = 0,
                    h = a.length - 1,
                    i = new Array(b);
                for (c = 0; h > c; c++) {
                    for (f = c, d = c + 1; h > d; d++) Math.abs(a[c][d]) > Math.abs(a[c][f]) && (f = d);
                    for (e = c; h + 1 > e; e++) g = a[e][c], a[e][c] = a[e][f], a[e][f] = g;
                    for (d = c + 1; h > d; d++)
                        for (e = h; e >= c; e--) a[e][d] -= a[e][c] * a[c][d] / a[c][c]
                }
                for (d = h - 1; d >= 0; d--) {
                    for (g = 0, e = d + 1; h > e; e++) g += a[e][d] * i[e];
                    i[d] = (a[h][d] - g) / a[d][d]
                }
                return i
            }

            function c(a, b) {
                var c = Math.pow(10, b);
                return Math.round(a * c) / c
            }
            var d, e = 2,
                f = {
                    linear: function(b, d, e) {
                        for (var f, g, h, i = [0, 0, 0, 0, 0], j = b.length, k = 0; j > k; k++) null !== b[k][1] && (i[0] += b[k][0], i[1] += b[k][1], i[2] += b[k][0] * b[k][0], i[3] += b[k][0] * b[k][1], i[4] += b[k][1] * b[k][1]);
                        return g = (j * i[3] - i[0] * i[1]) / (j * i[2] - i[0] * i[0]), h = i[1] / j - g * i[0] / j, f = b.map(function(a) {
                            var b = a[0];
                            return [b, g * b + h]
                        }), {
                            r2: a(b, f),
                            equation: [g, h],
                            points: f,
                            string: "y = " + c(g, e.precision) + "x + " + c(h, e.precision)
                        }
                    },
                    linearthroughorigin: function(b, d, e) {
                        for (var f, g, h = [0, 0], i = 0; i < b.length; i++) null !== b[i][1] && (h[0] += b[i][0] * b[i][0], h[1] += b[i][0] * b[i][1]);
                        return f = h[1] / h[0], g = b.map(function(a) {
                            var b = a[0];
                            return [b, f * b]
                        }), {
                            r2: a(b, g),
                            equation: [f],
                            points: g,
                            string: "y = " + c(f, e.precision) + "x"
                        }
                    },
                    exponential: function(b, d, e) {
                        for (var f, g, h, i, j = [0, 0, 0, 0, 0, 0], k = 0; k < b.length; k++) null !== b[k][1] && (j[0] += b[k][0], j[1] += b[k][1], j[2] += b[k][0] * b[k][0] * b[k][1], j[3] += b[k][1] * Math.log(b[k][1]), j[4] += b[k][0] * b[k][1] * Math.log(b[k][1]), j[5] += b[k][0] * b[k][1]);
                        return f = j[1] * j[2] - j[5] * j[5], g = Math.exp((j[2] * j[3] - j[5] * j[4]) / f), h = (j[1] * j[4] - j[5] * j[3]) / f, i = b.map(function(a) {
                            var b = a[0];
                            return [b, g * Math.exp(h * b)]
                        }), {
                            r2: a(b, i),
                            equation: [g, h],
                            points: i,
                            string: "y = " + c(g, e.precision) + "e^(" + c(h, e.precision) + "x)"
                        }
                    },
                    logarithmic: function(b, d, e) {
                        for (var f, g, h, i = [0, 0, 0, 0], j = b.length, k = 0; j > k; k++) null !== b[k][1] && (i[0] += Math.log(b[k][0]), i[1] += b[k][1] * Math.log(b[k][0]), i[2] += b[k][1], i[3] += Math.pow(Math.log(b[k][0]), 2));
                        return g = (j * i[1] - i[2] * i[0]) / (j * i[3] - i[0] * i[0]), f = (i[2] - g * i[0]) / j, h = b.map(function(a) {
                            var b = a[0];
                            return [b, f + g * Math.log(b)]
                        }), {
                            r2: a(b, h),
                            equation: [f, g],
                            points: h,
                            string: "y = " + c(f, e.precision) + " + " + c(g, e.precision) + " ln(x)"
                        }
                    },
                    power: function(b, d, e) {
                        for (var f, g, h, i = [0, 0, 0, 0], j = b.length, k = 0; j > k; k++) null !== b[k][1] && (i[0] += Math.log(b[k][0]), i[1] += Math.log(b[k][1]) * Math.log(b[k][0]), i[2] += Math.log(b[k][1]), i[3] += Math.pow(Math.log(b[k][0]), 2));
                        return g = (j * i[1] - i[2] * i[0]) / (j * i[3] - i[0] * i[0]), f = Math.exp((i[2] - g * i[0]) / j), h = b.map(function(a) {
                            var b = a[0];
                            return [b, f * Math.pow(b, g)]
                        }), {
                            r2: a(b, h),
                            equation: [f, g],
                            points: h,
                            string: "y = " + c(f, e.precision) + "x^" + c(g, e.precision)
                        }
                    },
                    polynomial: function(d, e, f) {
                        var g, h, i, j, k, l, m, n, o = [],
                            p = [],
                            q = 0,
                            r = 0,
                            s = d.length;
                        for (h = "undefined" == typeof e ? 3 : e + 1, i = 0; h > i; i++) {
                            for (k = 0; s > k; k++) null !== d[k][1] && (q += Math.pow(d[k][0], i) * d[k][1]);
                            for (o.push(q), q = 0, g = [], j = 0; h > j; j++) {
                                for (k = 0; s > k; k++) null !== d[k][1] && (r += Math.pow(d[k][0], i + j));
                                g.push(r), r = 0
                            }
                            p.push(g)
                        }
                        for (p.push(o), m = b(p, h), l = d.map(function(a) {
                                var b = a[0],
                                    c = m.reduce(function(a, c, d) {
                                        return a + c * Math.pow(b, d)
                                    }, 0);
                                return [b, c]
                            }), n = "y = ", i = m.length - 1; i >= 0; i--) n += i > 1 ? c(m[i], f.precision) + "x^" + i + " + " : 1 === i ? c(m[i], f.precision) + "x + " : c(m[i], f.precision);
                        return {
                            r2: a(d, l),
                            equation: m,
                            points: l,
                            string: n
                        }
                    },
                    lastvalue: function(b, d, e) {
                        for (var f = [], g = null, h = 0; h < b.length; h++) null !== b[h][1] && isFinite(b[h][1]) ? (g = b[h][1], f.push([b[h][0], b[h][1]])) : f.push([b[h][0], g]);
                        return {
                            r2: a(b, f),
                            equation: [g],
                            points: f,
                            string: "" + c(g, e.precision)
                        }
                    }
                };
            return d = function(a, b, c, d) {
                var g = "object" == typeof c && "undefined" == typeof d ? c : d || {};
                return g.precision || (g.precision = e), "string" == typeof a ? f[a.toLowerCase()](b, c, g) : null
            }
        });
        /*** end of Regression.JS **/