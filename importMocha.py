import nuke, os

def importMocha():
    filename = nuke.getFilename("Mocha tracking data", "*.txt")
    f = open(filename)
    row = -1
    active = False
    data = []
    height = 0
    for l in f.readlines():
        items = l.split()
        if len(items) < 1:
            active = False

        if l.lower().lstrip().startswith("source height"):
            height = float(items[2])

        if active:
            data[row].append(items)
            
        if l.lower().lstrip().startswith("frame"):
            row += 1
            active = True
            data.append([])

    cornerPinNode = nuke.createNode("CornerPin2D")
    cornerPinReverseNode = nuke.createNode("CornerPin2D")
    points = ["1", "2", "4", "3"]
    for c in range(4):
        #cornerPinNode.knob("to" + str(c + 1)).setAnimated(True)
        toKnob = cornerPinNode.knob("to" + points[c])
        fromKnob = cornerPinReverseNode.knob("from" + points[c])
        for k in (toKnob, fromKnob):
            k.setAnimated(0, True)
            k.setAnimated(1, True)
        
            for (f, x, y) in data[c]:
                k.setValueAt(float(x), float(f), 0)
                k.setValueAt(height - float(y), float(f), 1)

