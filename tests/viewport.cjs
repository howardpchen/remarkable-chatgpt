// Exercise the production QML calibration function with synthetic view maps.
const fs = require('fs');
const vm = require('vm');
const assert = require('assert');
const source = fs.readFileSync(__dirname + '/../src/inserter.inc', 'utf8');
const fn = source.split('    function viewportSnapshot(')[1].split('\n    function insertText(')[0];
const context = {};
vm.createContext(context);
vm.runInContext('function viewportSnapshot(' + fn, context);
function view(width, height, scale = 1, x = 0, y = 0) {
    return {width, height,
        mapFromItem: (_, a, b) => ({x:a, y:b}),
        viewToScene: p => ({x:p.x * scale + x, y:p.y * scale + y})};
}
for (const [width, height] of [[1404,1872], [1620,2160], [954,1696]]) {
    const original = context.viewportSnapshot(view(width, height));
    assert(original);
    assert.equal(original, context.viewportSnapshot(view(width, height)));
    assert.notEqual(original, context.viewportSnapshot(view(width, height, 0.5)));
    assert.notEqual(original, context.viewportSnapshot(view(width, height, 1, 10, 50)));
    assert.notEqual(original, context.viewportSnapshot(view(height, width)));
}
assert.equal(context.viewportSnapshot(null), '');
assert.equal(context.viewportSnapshot(view(0, 100)), '');
assert.equal(context.viewportSnapshot(view(Infinity, 100)), '');
assert.equal(context.viewportSnapshot(view(100, 100, 0)), '');
assert.equal(context.viewportSnapshot(view(100, 100, NaN)), '');
assert.equal(context.viewportSnapshot({width:100,height:100}), '');
console.log('PASS: viewport snapshots detect zoom, pan and resize; invalid maps fail closed');
