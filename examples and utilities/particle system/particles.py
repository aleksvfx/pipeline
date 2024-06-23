# -*- coding: utf-8 -*-
from __future__ import with_statement
import nuke
import _nukemath
import random

# -----------------------------------------------------------------------------

#def inc(v):
#    v[0] += 1

# -----------------------------------------------------------------------------

def ParticlesCallback():
    n = nuke.thisNode()
    k = nuke.thisKnob()
    if (k.name() == 'colorInterpolation'):
        if (k.value() == 'none'):
            n['deathColor'].setEnabled(False)
            n['deathColorVariance'].setEnabled(False)
        else:
            n['deathColor'].setEnabled(True)
            n['deathColorVariance'].setEnabled(True)
        if (k.value() == 'custom'):
            n['customColorCurve'].setEnabled(True)
            n['customColorCurve'].setVisible(True)
        else:
            n['customColorCurve'].setEnabled(False)
            n['customColorCurve'].setVisible(False)
    elif (k.name() == 'transparencyInterpolation'):
        if (k.value() == 'none'):
            n['deathTransparency'].setEnabled(False)
            n['deathTransparencyVariance'].setEnabled(False)
        else:
            n['deathTransparency'].setEnabled(True)
            n['deathTransparencyVariance'].setEnabled(True)
        if (k.value() == 'custom'):
            n['customTransparencyCurve'].setEnabled(True)
            n['customTransparencyCurve'].setVisible(True)
        else:
            n['customTransparencyCurve'].setEnabled(False)
            n['customTransparencyCurve'].setVisible(False)
    elif (k.name() == 'sizeInterpolation'):
        if (k.value() == 'none'):
            n['deathSize'].setEnabled(False)
            n['deathSizeVariance'].setEnabled(False)
        else:
            n['deathSize'].setEnabled(True)
            n['deathSizeVariance'].setEnabled(True)
        if (k.value() == 'custom'):
            n['customSizeCurve'].setEnabled(True)
            n['customSizeCurve'].setVisible(True)
        else:
            n['customSizeCurve'].setEnabled(False)
            n['customSizeCurve'].setVisible(False)
    elif (k.name() == 'emitterType'):
        if (k.value() == 'Points from OBJ'):
            n['objFileName'].setVisible(True)
        else:
            n['objFileName'].setVisible(False)
    elif (k.name() == 'objFileName'):
        with nuke.thisNode():
            nuke.toNode('objVisualization')['file'].setValue(k.value())
    elif (k.name() == 'applyMaterial'):
        ApplyMaterials(k.value())
    elif (k.name() == 'firstImg'):
        ImageSwitch(not k.value())
    elif (k.name() == 'cacheTextures'):
        CacheTextures(k.value())
    elif (k.name() == 'showPanel'):
        if (n['sizeInterpolation'].value() != 'custom'):
            n['customSizeCurve'].setEnabled(False)
            n['customSizeCurve'].setVisible(False)
        if (n['colorInterpolation'].value() != 'custom'):
            n['customColorCurve'].setEnabled(False)
            n['customColorCurve'].setVisible(False)
        if (n['transparencyInterpolation'].value() != 'custom'):
            n['customTransparencyCurve'].setEnabled(False)
            n['customTransparencyCurve'].setVisible(False)
        if (n['emitterType'].value() != 'Points from OBJ'):
            n['objFileName'].setVisible(False)

# -----------------------------------------------------------------------------
            
def ChooseImageSource(img1Connected, img2Connected, img3Connected, random):
    if not (img1Connected or img2Connected or img3Connected) : return 3
    connectedInputs = []
    if (img1Connected): connectedInputs.append(0)
    if (img2Connected): connectedInputs.append(1)
    if (img3Connected): connectedInputs.append(2)
    return connectedInputs[int(random * len(connectedInputs))]

# -----------------------------------------------------------------------------

def GetRandomSeed(seedOffset = 0, seedScale = 1):
    return '(parent.randomSeed + ' + str(seedOffset) + ') * ' + str(seedScale)

# -----------------------------------------------------------------------------

def GetRandomSeedTriple(seedOffset = 0, seedScale = 1):
    randomSeed = GetRandomSeed(seedOffset, seedScale)
    return randomSeed + ', ' + randomSeed + ', ' + randomSeed

# -----------------------------------------------------------------------------

def GetOBJVertices (objFileName):
    vertices = []
    
    file = None
    try:
        file = open(objFileName, 'r')
        for line in file:
            words = line.split()
            if words[0] == 'v':
                x, y, z = float(words[1]), float(words[2]), float(words[3])
                vertices.append(_nukemath.Vector3(x,y,z))
    except IOError:
        nuke.message('Error while processing the file ' + objFileName)
    finally:
        if (file != None): file.close()

    return vertices

# -----------------------------------------------------------------------------

def ApplyMaterials(apply):
    curNode = 0
    if (apply):
        ApplyMaterials(False)
        n = nuke.toNode('color' + str(curNode))
        while (n != None):
            applyMaterial = nuke.nodes.ApplyMaterial(label='auto',name='applyMaterial'+str(curNode))
            applyMaterial.setInput(0, nuke.toNode('geoSwitch'))
            n2 = nuke.toNode('diskCache' + str(curNode))
            if (n2 != None):
                applyMaterial.setInput(1, n2)
            else:
                applyMaterial.setInput(1, n)
            applyMaterial['disable'].setExpression('!particleRoot' + str(curNode) +'.alive')
            nuke.toNode('transform' + str(curNode)).setInput(0, applyMaterial)
            nuke.autoplace(applyMaterial)

            curNode += 1
            n = nuke.toNode('color' + str(curNode))
    else:
        n = nuke.toNode('applyMaterial' + str(curNode))
        while (n != None):
            nuke.delete(n)
            curNode += 1
            n = nuke.toNode('applyMaterial' + str(curNode))

# -----------------------------------------------------------------------------

def CacheTextures(cache):
    curNode = 0
    if (cache):
        CacheTextures(False)
        n = nuke.toNode('color' + str(curNode))
        while (n != None):
            cache = nuke.nodes.DiskCache(label='auto',name='diskCache'+str(curNode))
            cache.setInput(0, n)
            cache['disable'].setExpression('!particleRoot' + str(curNode) +'.alive')
            applyMaterial = nuke.toNode('applyMaterial' + str(curNode))
            if (applyMaterial != None):
                applyMaterial.setInput(1, cache)
            nuke.autoplace(cache)

            curNode += 1
            n = nuke.toNode('color' + str(curNode))
    else:
        n = nuke.toNode('diskCache' + str(curNode))
        while (n != None):
            nuke.delete(n)
            curNode += 1
            n = nuke.toNode('diskCache' + str(curNode))

# -----------------------------------------------------------------------------

def ImageSwitch(enable):
    curNode = 0
    if (enable):
        ImageSwitch(False)
        n = nuke.toNode('particleRoot' + str(curNode))
        while (n != None):
	  imageSwitch = nuke.nodes.Switch(label='auto',name='imageSwitch'+str(curNode))
	  imageSwitch.setInput(0, nuke.toNode('img1Shuffle'))
	  imageSwitch.setInput(1, nuke.toNode('img2Shuffle'))
	  imageSwitch.setInput(2, nuke.toNode('img3Shuffle'))
	  imageSwitch.setInput(3, nuke.toNode('defaultTextureReformat'))
	  imageSwitch['which'].setExpression('[python particles.ChooseImageSource(nuke.thisNode().input(0).input(0).knob("connected").value(),nuke.thisNode().input(1).input(0).knob("connected").value(),nuke.thisNode().input(2).input(0).knob("connected").value(),' + str(random.random()) + ')]')
	  n.setInput(0,imageSwitch)

          nuke.autoplace(imageSwitch)

          curNode += 1
          n = nuke.toNode('particleRoot' + str(curNode))
    else:
        n = nuke.toNode('imageSwitch' + str(curNode))
        while (n != None):
            nuke.delete(n)
	    nuke.toNode('particleRoot' + str(curNode)).setInput(0, nuke.toNode('img1Shuffle'))
            curNode += 1
            n = nuke.toNode('imageSwitch' + str(curNode))

# -----------------------------------------------------------------------------

def MotionEquation(startPosition, startVelocity, acceleration, time):
    return startPosition.__add__((startVelocity.__mul__(time)).__add__(acceleration.__mul__(0.5 * time * time)))

# -----------------------------------------------------------------------------

def Update(transformNode, gizmoNode, frame, bakeSimulation = False, onlyUpdateIfAlive = True):
    #undo = nuke.Undo()
    #undo.name('Update Particles')
    #undo.begin()
    #undo.disable()

    #print (str(transformNode.name()) + ' ' + str(gizmoNode.name()) + ' ' + str(frame) + ' ' + str(bakeSimulation))

    fps = nuke.root().fps()

    # update only if the current particle is alive
    if ((not onlyUpdateIfAlive) or nuke.toNode(transformNode['particleRoot'].value())['alive'].valueAt(frame)):
        position = _nukemath.Vector3(transformNode['transformedStartPosition'].x(),transformNode['transformedStartPosition'].y(),transformNode['transformedStartPosition'].z())

        birth = nuke.toNode(transformNode['particleRoot'].value())['birth'].value()
        death = nuke.toNode(transformNode['particleRoot'].value())['death'].value()
        frameRange = range(int(birth),frame)

        acceleration = _nukemath.Vector3(0,0,0)
        velocity = _nukemath.Vector3(transformNode['startVelocity'].valueAt(birth,0),transformNode['startVelocity'].valueAt(birth,1),transformNode['startVelocity'].valueAt(birth,2)).__div__(fps).__div__(transformNode['mass'].valueAt(birth))
        turbulence = _nukemath.Vector3(0,0,0)

        # integrate from birth to current frame
        for f in frameRange:
            mass = transformNode['mass'].valueAt(f)
            gravity = _nukemath.Vector3(gizmoNode['gravity'].valueAt(f,0),gizmoNode['gravity'].valueAt(f,1),gizmoNode['gravity'].valueAt(f,2)).__div__(fps).__mul__(0.25 * mass) # mass multiplication trick to strengthen the effect of air resistance
            externalForce = _nukemath.Vector3(gizmoNode['externalForce'].valueAt(f,0),gizmoNode['externalForce'].valueAt(f,1),gizmoNode['externalForce'].valueAt(f,2)).__div__(fps)
            drag = velocity.__mul__(-gizmoNode['drag'].valueAt(f))

            turbulenceAmount = _nukemath.Vector3(gizmoNode['turbulenceAmount'].valueAt(f,0), gizmoNode['turbulenceAmount'].valueAt(f,1), gizmoNode['turbulenceAmount'].valueAt(f,2))

            # apply simple equation if parameters stay constant and no turbulence or collision have to be considered on the way
            if (gizmoNode['parametersConstant'].valueAt(f) and (turbulenceAmount.lengthSquared() < 0.01) and (not gizmoNode['plane1Activate'].valueAt(f))):
                if (not bakeSimulation):
                    if (f == int(birth)):
                        p = MotionEquation(position, velocity, gravity.__add__((externalForce.__add__(drag)).__div__(mass)), frame - birth)
                        transformNode['translate'].setValue(p.x,0)
                        transformNode['translate'].setValue(p.y,1)
                        transformNode['translate'].setValue(p.z,2)
                        return 0
##                else:
##                        p = MotionEquation(position, velocity, gravity.__add__((externalForce.__add__(drag)).__div__(mass)), f - birth)
##                        transformNode['translate'].setValue(p.x,0)
##                        transformNode['translate'].setValue(p.y,1)
##                        transformNode['translate'].setValue(p.z,2)
##                        transformNode['translate'].setKeyAt(f)
##                        continue

            if (turbulenceAmount.lengthSquared() > 0.01):
                randomSeed = gizmoNode['randomSeed'].valueAt(f) + 10 * transformNode['particleNumber'].value()
                turbulence = _nukemath.Vector3(\
                    turbulenceAmount.x * (nuke.expr('random(' + str(position.x * gizmoNode['turbulenceFrequency'].valueAt(f,0)) + ',' + str(randomSeed) + ',' + str(randomSeed) + ')') - 0.5),\
                    turbulenceAmount.y * (nuke.expr('random(' + str(randomSeed) + ',' + str(position.y * gizmoNode['turbulenceFrequency'].valueAt(f,1)) + ',' + str(randomSeed) + ')') - 0.5),\
                    turbulenceAmount.z * (nuke.expr('random(' + str(randomSeed) + ',' + str(randomSeed) + ',' + str(position.z * gizmoNode['turbulenceFrequency'].valueAt(f,2)) + ')') - 0.5)).__div__(fps)

            acceleration = gravity.__add__(((externalForce.__add__(turbulence)).__add__(drag)).__div__(mass))
            velocity = velocity.__add__(acceleration)

            oldPosition = _nukemath.Vector3(position.x, position.y, position.z)
            position = position.__add__(velocity)

            if (gizmoNode['plane1Activate'].valueAt(f)):
                n = _nukemath.Vector3(gizmoNode['plane1NormalizedNormal'].valueAt(f,0), gizmoNode['plane1NormalizedNormal'].valueAt(f,1), gizmoNode['plane1NormalizedNormal'].valueAt(f,2))
                d = gizmoNode['plane1DistanceFromOrigin'].valueAt(f) - gizmoNode['collisionRadius'].valueAt(f)
                if ((_nukemath.Vector3.distanceFromPlane(position, n.x, n.y, n.z, d)) < 0):
                    u = (n.x * oldPosition.x + n.y * oldPosition.y + n.z * oldPosition.z + d) / (n.x * (oldPosition.x - position.x) + n.y * (oldPosition.y - position.y) + n.z * (oldPosition.z - position.z))
                    position = oldPosition.__add__(velocity.__mul__(u))
                    velocity = (velocity.__sub__(n.__mul__(2 * n.dot(velocity)))).__mul__(gizmoNode['bounceAmount'].valueAt(f))

            if (bakeSimulation):
                transformNode['translate'].setValue(position.x,0,f)
                transformNode['translate'].setValue(position.y,1,f)
                transformNode['translate'].setValue(position.z,2,f)
                transformNode['translate'].setKeyAt(f)

        if (not bakeSimulation):
            transformNode['translate'].setValue(position.x,0)
            transformNode['translate'].setValue(position.y,1)
            transformNode['translate'].setValue(position.z,2)
        #undo.cancel()
        #undo.end()
    
    return 0

# -----------------------------------------------------------------------------

def ResetTransformExpressions(node, clear):
    if (clear):
        node['transformedStartPosition'].clearAnimated()
        node['transformedStartPosition2'].clearAnimated()
        node['startVelocity'].clearAnimated()
        node['mass'].clearAnimated()
    else:
        node['transformedStartPosition'].setExpression(node['transformedStartPositionExpressionX'].value(),0)
        node['transformedStartPosition'].setExpression(node['transformedStartPositionExpressionY'].value(),1)
        node['transformedStartPosition'].setExpression(node['transformedStartPositionExpressionZ'].value(),2)
        node['transformedStartPosition2'].setExpression(node['transformedStartPosition2ExpressionX'].value(),0)
        node['transformedStartPosition2'].setExpression(node['transformedStartPosition2ExpressionY'].value(),1)
        node['transformedStartPosition2'].setExpression(node['transformedStartPosition2ExpressionZ'].value(),2)
        node['startVelocity'].setExpression(node['startVelocityExpressionX'].value(),0)
        node['startVelocity'].setExpression(node['startVelocityExpressionY'].value(),1)
        node['startVelocity'].setExpression(node['startVelocityExpressionZ'].value(),2)
        node['mass'].setExpression(node['massExpression'].value())

# -----------------------------------------------------------------------------

def BakeSimulation(bake):
    nuke.thisNode()['unbakeSimulation'].setEnabled(bake)

    curNode = 0
    n = nuke.toNode('transform' + str(curNode))
    while (n != None):
        if (bake):
            ResetTransformExpressions(n, False)
            n['pivot'].clearAnimated(0)
            n['translate'].setAnimated()
            Update(n, nuke.thisNode(), int(nuke.toNode(n['particleRoot'].value())['birth'].value() + nuke.toNode(n['particleRoot'].value())['death'].value() + 1), True, False)
            ResetTransformExpressions(n, True)
        else:
            ResetTransformExpressions(n, False)
            n['translate'].clearAnimated()
            n['pivot'].setExpression('0 * (!' + n['particleRoot'].value() + '.alive || [ python particles.Update(nuke.thisNode(), nuke.thisParent(), nuke.frame()) ])',0)
        curNode += 1
        n = nuke.toNode('transform' + str(curNode))

# -----------------------------------------------------------------------------

def Generate():
    #try:k
        undo = nuke.Undo()
        undo.name('Generate Particles')
        undo.begin()

        nuke.thisNode()['unbakeSimulation'].setEnabled(False)

        this = nuke.thisNode()

        with this:
            # delete old nodes
            for n in nuke.allNodes():
                if (n['label'].value() == 'auto'): nuke.delete(n)

            # load OBJ
            objVertices = []
            if (this['emitterType'].value() == 'Points from OBJ'):
                objVertices = GetOBJVertices(this['objFileName'].value())
            objVertexCount = len(objVertices)
            vertexIndices = range(objVertexCount)

            # generate particles
            frameRange = range(int(nuke.root().firstFrame() - this['prerollTime'].value() * nuke.root().fps()), nuke.root().lastFrame() + 1)
            curParticle = 0
            scene = None
            curSceneCount = 0
            exactParticlesAtFrame = 0.9999
            for f in frameRange:
                exactParticlesAtFrame += this['particlesPerSecond'].valueAt(f) / nuke.root().fps()
                if (int(exactParticlesAtFrame) - curParticle) > 0:
                    for p in range(int(exactParticlesAtFrame) - curParticle):

                        random.seed(int((this['randomSeed'].value()+curParticle+1)*(curParticle+1)))

                        # image switch
                        if (not this['firstImg'].value()):
                            imageSwitch = nuke.nodes.Switch(label='auto',name='imageSwitch'+str(curParticle))
                            imageSwitch.setInput(0, nuke.toNode('img1Shuffle'))
                            imageSwitch.setInput(1, nuke.toNode('img2Shuffle'))
                            imageSwitch.setInput(2, nuke.toNode('img3Shuffle'))
                            imageSwitch.setInput(3, nuke.toNode('defaultTextureReformat'))
                            imageSwitch['which'].setExpression('[python particles.ChooseImageSource(nuke.thisNode().input(0).input(0).knob("connected").value(),nuke.thisNode().input(1).input(0).knob("connected").value(),nuke.thisNode().input(2).input(0).knob("connected").value(),' + str(random.random()) + ')]')

                        # particle root dot
                        dot = nuke.nodes.Dot(label='auto',name='particleRoot'+str(curParticle))
                        if (not this['firstImg'].value()):
                            dot.setInput(0, imageSwitch)
                        else:
                            dot.setInput(0, nuke.toNode('img1Shuffle'))

                        # calculate life and interpolation curves
                        lifeKnob = nuke.Double_Knob('life','life')
                        dot.addKnob(lifeKnob)
                        lifeKnob.setExpression('(1 + 2*parent.lifeVariance*' + str((random.random()-0.5)) + ')*root.fps*parent.life')
                        lifeKnob.setVisible(False)

                        birthKnob = nuke.Double_Knob('birth','birth')
                        dot.addKnob(birthKnob)
                        birthKnob.setExpression(str(f))
                        birthKnob.setVisible(False)

                        deathKnob = nuke.Double_Knob('death','death')
                        dot.addKnob(deathKnob)
                        deathKnob.setExpression('birth + life')
                        deathKnob.setVisible(False)

                        aliveKnob = nuke.Int_Knob('alive','alive')
                        dot.addKnob(aliveKnob)
                        aliveKnob.setExpression('inrange(frame,particleRoot' + str(curParticle) + '.birth,particleRoot' + str(curParticle) + '.death)')
                        aliveKnob.setVisible(False)

                        linearInterpolationKnob = nuke.Double_Knob('linearInterpolation','linearInterpolation')
                        dot.addKnob(linearInterpolationKnob)
                        linearInterpolationKnob.setExpression('min(1,max(0,1 - (frame - birth) / (death - birth)))')
                        linearInterpolationKnob.setVisible(False)

                        cosInterpolationKnob = nuke.Double_Knob('cosInterpolation','cosInterpolation')
                        dot.addKnob(cosInterpolationKnob)
                        cosInterpolationKnob.setExpression('1 - 0.5 * (cos(linearInterpolation*pi) + 1)')
                        cosInterpolationKnob.setVisible(False)

                        colorInterpolationKnob = nuke.Double_Knob('colorInterpolationValue','colorInterpolationValue')
                        dot.addKnob(colorInterpolationKnob)
                        colorInterpolationKnob.setExpression('(parent.colorInterpolation == 0) + (parent.colorInterpolation == 1) * particleRoot' + str(curParticle) + '.linearInterpolation + (parent.colorInterpolation == 2) * particleRoot' + str(curParticle) + '.cosInterpolation  + (parent.colorInterpolation == 3) * (1 - [python nuke.thisParent().knob("customColorCurve").valueAt(1 + (nuke.frame() - nuke.thisNode().knob("birth").value()) / (nuke.thisNode().knob("death").value() - nuke.thisNode().knob("birth").value()))])')
                        colorInterpolationKnob.setVisible(False)

                        transparencyInterpolationKnob = nuke.Double_Knob('transparencyInterpolationValue','transparencyInterpolationValue')
                        dot.addKnob(transparencyInterpolationKnob)
                        transparencyInterpolationKnob.setExpression('(parent.transparencyInterpolation == 0) + (parent.transparencyInterpolation == 1) * particleRoot' + str(curParticle) + '.linearInterpolation + (parent.transparencyInterpolation == 2) * particleRoot' + str(curParticle) + '.cosInterpolation  + (parent.transparencyInterpolation == 3) * (1 - [python nuke.thisParent().knob("customTransparencyCurve").valueAt(1 + (nuke.frame() - nuke.thisNode().knob("birth").value()) / (nuke.thisNode().knob("death").value() - nuke.thisNode().knob("birth").value()))])')
                        transparencyInterpolationKnob.setVisible(False)
                        

                        transparencyKnob = nuke.Double_Knob('transparency','transparency')
                        dot.addKnob(transparencyKnob)
                        transparencyKnob.setExpression('((1+2*parent.birthTransparencyVariance*(pow(' + str(random.random()) + ',1+parent.sRGBgammaTransparency*1.2)-0.5)) * particleRoot' + str(curParticle) + '.transparencyInterpolationValue * (parent.birthTransparency) + (1+2*parent.deathTransparencyVariance*(pow(' + str(random.random()) + ',1+parent.sRGBgammaTransparency*1.2)-0.5)) * (1 - particleRoot' + str(curParticle) + '.transparencyInterpolationValue) * (parent.deathTransparency))')
                        transparencyKnob.setVisible(False)

                        sizeInterpolationKnob = nuke.Double_Knob('sizeInterpolationValue','sizeInterpolationValue')
                        dot.addKnob(sizeInterpolationKnob)
                        sizeInterpolationKnob.setExpression('(parent.sizeInterpolation == 0) + (parent.sizeInterpolation == 1) * particleRoot' + str(curParticle) + '.linearInterpolation + (parent.sizeInterpolation == 2) * particleRoot' + str(curParticle) + '.cosInterpolation + (parent.sizeInterpolation == 3) * (1 - [python nuke.thisParent().knob("customSizeCurve").valueAt(1 + (nuke.frame() - nuke.thisNode().knob("birth").value()) / (nuke.thisNode().knob("death").value() - nuke.thisNode().knob("birth").value()))])')
                        sizeInterpolationKnob.setVisible(False)

                        # texture frame offset
                        timeShift = nuke.nodes.TimeOffset(label='auto',name='timeOffset'+str(curParticle))
                        timeShift.setInput(0, dot)
                        timeShift['time_offset'].setExpression('-' + str(random.random()) + ' * (parent.maxFrameShift + 1)')
                        timeShift['disable'].setExpression('!particleRoot' + str(curParticle) +'.alive')

                        # premultiplied color + alpha
                        multiply = nuke.nodes.Multiply(label='auto',name='color'+str(curParticle))
                        multiply.setInput(0, timeShift)
                        multiply['value'].setSingleValue(False)
                        multiply['channels'].setValue('rgba')

                        multiply['value'].setExpression('(particleRoot' + str(curParticle) +'.transparency * ((1+2*parent.birthColorVariance.r*(pow(' + str(random.random()) + ',1+parent.sRGBgammaColor*1.2)-0.5)) * particleRoot' + str(curParticle) + '.colorInterpolationValue * (parent.birthColor.r) + (1+2*parent.deathColorVariance.r*(pow(' + str(random.random()) + ',1+parent.sRGBgammaColor*1.2)-0.5)) * (1 - particleRoot' + str(curParticle) + '.colorInterpolationValue) * (parent.deathColor.r)))',0)
                        multiply['value'].setExpression('(particleRoot' + str(curParticle) +'.transparency * ((1+2*parent.birthColorVariance.g*(pow(' + str(random.random()) + ',1+parent.sRGBgammaColor*1.2)-0.5)) * particleRoot' + str(curParticle) + '.colorInterpolationValue * (parent.birthColor.g) + (1+2*parent.deathColorVariance.g*(pow(' + str(random.random()) + ',1+parent.sRGBgammaColor*1.2)-0.5)) * (1 - particleRoot' + str(curParticle) + '.colorInterpolationValue) * (parent.deathColor.g)))',1)
                        multiply['value'].setExpression('(particleRoot' + str(curParticle) +'.transparency * ((1+2*parent.birthColorVariance.b*(pow(' + str(random.random()) + ',1+parent.sRGBgammaColor*1.2)-0.5)) * particleRoot' + str(curParticle) + '.colorInterpolationValue * (parent.birthColor.b) + (1+2*parent.deathColorVariance.b*(pow(' + str(random.random()) + ',1+parent.sRGBgammaColor*1.2)-0.5)) * (1 - particleRoot' + str(curParticle) + '.colorInterpolationValue) * (parent.deathColor.b)))',2)
                        
                        multiply['value'].setExpression('particleRoot' + str(curParticle) +'.transparency',3)
                        
                        multiply['disable'].setExpression('!particleRoot' + str(curParticle) +'.alive')

                        cache = None
                        if (this['cacheTextures'].value()):
                            cache = nuke.nodes.DiskCache(label='auto',name='diskCache'+str(curParticle))
                            cache.setInput(0, multiply)
                            cache['disable'].setExpression('!particleRoot' + str(curParticle) +'.alive')

                        if (this['applyMaterial'].value()):
                            applyMaterial = nuke.nodes.ApplyMaterial(label='auto',name='applyMaterial'+str(curParticle))
                            applyMaterial.setInput(0, nuke.toNode('geoSwitch'))
                            if (this['cacheTextures'].value()):
                                applyMaterial.setInput(1, cache)
                            else:
                                applyMaterial.setInput(1, multiply)
                            applyMaterial['disable'].setExpression('!particleRoot' + str(curParticle) +'.alive')

                        # transform
                        transform = nuke.nodes.TransformGeo(label='auto',name='transform'+str(curParticle))
                        transform['selectable'].setValue(False)
                        if (this['applyMaterial'].value()):
                            transform.setInput(0, nuke.toNode('applyMaterial' + str(curParticle)))
                        else:
                            transform.setInput(0, nuke.toNode('geoSwitch'))
                        transform.setInput(2, nuke.toNode('lookat'))
                        transform['disable'].setExpression('!particleRoot' + str(curParticle) +'.alive')

                        # start position
                        startPosition = _nukemath.Vector3()
                        #startPositionKnob = nuke.XYZ_Knob('startPosition','startPosition')
                        #transform.addKnob(startPositionKnob)
                        if (this['emitterType'].value() == 'Point'):
                            startPosition.x = 0
                            startPosition.y = 0
                            startPosition.z = 0
                        elif (this['emitterType'].value() == 'Sphere'):
                            v = _nukemath.Vector3(random.random()-0.5,random.random()-0.5,random.random()-0.5)
                            while (v.lengthSquared() > 0.25):
                                v = _nukemath.Vector3(random.random()-0.5,random.random()-0.5,random.random()-0.5)
                            startPosition.x = v.x
                            startPosition.y = v.y
                            startPosition.z = v.z
                        elif (this['emitterType'].value() == 'Box'):
                            startPosition.x = random.random()-0.5
                            startPosition.y = random.random()-0.5
                            startPosition.z = random.random()-0.5
                        else:
                            indexListPosition = int(len(vertexIndices) * random.random())
                            vertexIndex = vertexIndices[indexListPosition]
                            del vertexIndices[indexListPosition]
                            if (len(vertexIndices) == 0):
                                vertexIndices = range(objVertexCount)
                            startPosition.x = objVertices[vertexIndex].x
                            startPosition.y = objVertices[vertexIndex].y
                            startPosition.z = objVertices[vertexIndex].z
                        #startPositionKnob.setVisible(False)

                        # transformed start position
                        transformedStartPositionKnob = nuke.XYZ_Knob('transformedStartPosition','transformedStartPosition')
                        transform.addKnob(transformedStartPositionKnob)
                        translate = '[python nuke.thisParent().knob("position").valueAt(nuke.toNode("particleRoot' + str(curParticle) + '").knob("birth").value())\[0\]]'
                        scale = '[python nuke.thisParent().knob("scale").valueAt(nuke.toNode("particleRoot' + str(curParticle) + '").knob("birth").value())\[0\]]'
                        expressionXKnob = nuke.String_Knob('transformedStartPositionExpressionX','transformedStartPositionExpressionX')
                        transform.addKnob(expressionXKnob)
                        expressionXKnob.setVisible(False)
                        expressionXKnob.setValue('(' + str(startPosition.x) + ' * ' + scale + ' + ' + translate + ')')
                        transformedStartPositionKnob.setExpression(expressionXKnob.value(),0)
                        
                        translate = '[python nuke.thisParent().knob("position").valueAt(nuke.toNode("particleRoot' + str(curParticle) + '").knob("birth").value())\[1\]]'
                        scale = '[python nuke.thisParent().knob("scale").valueAt(nuke.toNode("particleRoot' + str(curParticle) + '").knob("birth").value())\[1\]]'
                        expressionYKnob = nuke.String_Knob('transformedStartPositionExpressionY','transformedStartPositionExpressionY')
                        transform.addKnob(expressionYKnob)
                        expressionYKnob.setVisible(False)
                        expressionYKnob.setValue('(' + str(startPosition.y) + ' * ' + scale + ' + ' + translate + ')')
                        transformedStartPositionKnob.setExpression(expressionYKnob.value(),1)
                        
                        translate = '[python nuke.thisParent().knob("position").valueAt(nuke.toNode("particleRoot' + str(curParticle) + '").knob("birth").value())\[2\]]'
                        scale = '[python nuke.thisParent().knob("scale").valueAt(nuke.toNode("particleRoot' + str(curParticle) + '").knob("birth").value())\[2\]]'
                        expressionZKnob = nuke.String_Knob('transformedStartPositionExpressionZ','transformedStartPositionExpressionZ')
                        transform.addKnob(expressionZKnob)
                        expressionZKnob.setVisible(False)
                        expressionZKnob.setValue('(' + str(startPosition.z) + ' * ' + scale + ' + ' + translate + ')')
                        transformedStartPositionKnob.setExpression(expressionZKnob.value(),2)

                        # transformed start position one frame earlier
                        transformedStartPositionKnob2 = nuke.XYZ_Knob('transformedStartPosition2','transformedStartPosition2')
                        transform.addKnob(transformedStartPositionKnob2)
                        translate = '[python nuke.thisParent().knob("position").valueAt(nuke.toNode("particleRoot' + str(curParticle) + '").knob("birth").value()-1)\[0\]]'
                        scale = '[python nuke.thisParent().knob("scale").valueAt(nuke.toNode("particleRoot' + str(curParticle) + '").knob("birth").value()-1)\[0\]]'
                        expressionXKnob = nuke.String_Knob('transformedStartPosition2ExpressionX','transformedStartPosition2ExpressionX')
                        transform.addKnob(expressionXKnob)
                        expressionXKnob.setVisible(False)
                        expressionXKnob.setValue('(' + str(startPosition.x) + ' * ' + scale + ' + ' + translate + ')')
                        transformedStartPositionKnob2.setExpression(expressionXKnob.value(),0)

                        translate = '[python nuke.thisParent().knob("position").valueAt(nuke.toNode("particleRoot' + str(curParticle) + '").knob("birth").value()-1)\[1\]]'
                        scale = '[python nuke.thisParent().knob("scale").valueAt(nuke.toNode("particleRoot' + str(curParticle) + '").knob("birth").value()-1)\[1\]]'
                        expressionYKnob = nuke.String_Knob('transformedStartPosition2ExpressionY','transformedStartPosition2ExpressionY')
                        transform.addKnob(expressionYKnob)
                        expressionYKnob.setVisible(False)
                        expressionYKnob.setValue('(' + str(startPosition.y) + ' * ' + scale + ' + ' + translate + ')')
                        transformedStartPositionKnob2.setExpression(expressionYKnob.value(),1)

                        translate = '[python nuke.thisParent().knob("position").valueAt(nuke.toNode("particleRoot' + str(curParticle) + '").knob("birth").value()-1)\[2\]]'
                        scale = '[python nuke.thisParent().knob("scale").valueAt(nuke.toNode("particleRoot' + str(curParticle) + '").knob("birth").value()-1)\[2\]]'
                        expressionZKnob = nuke.String_Knob('transformedStartPosition2ExpressionZ','transformedStartPosition2ExpressionZ')
                        transform.addKnob(expressionZKnob)
                        expressionZKnob.setVisible(False)
                        expressionZKnob.setValue('(' + str(startPosition.z) + ' * ' + scale + ' + ' + translate + ')')
                        transformedStartPositionKnob2.setExpression(expressionXKnob.value(),2)

			transformedStartPositionKnob.setVisible(False)
			transformedStartPositionKnob2.setVisible(False)

                        # start velocity force
                        startVelocityKnob = nuke.XYZ_Knob('startVelocity','startVelocity')
                        transform.addKnob(startVelocityKnob)

                        expressionXKnob = nuke.String_Knob('startVelocityExpressionX','startVelocityExpressionX')
                        transform.addKnob(expressionXKnob)
                        expressionXKnob.setVisible(False)
                        expressionXKnob.setValue('(1+2*parent.velocityVariance.x*' + str((random.random() - 0.5)) + ') * parent.startVelocity.x + 2 * ' + str((random.random() - 0.5)) + ' * parent.maxExtraVelocity.x + parent.inherentEmitterVelocity * (transformedStartPosition.x - transformedStartPosition2.x)')
                        startVelocityKnob.setExpression(expressionXKnob.value(),0)

                        expressionYKnob = nuke.String_Knob('startVelocityExpressionY','startVelocityExpressionY')
                        transform.addKnob(expressionYKnob)
                        expressionYKnob.setVisible(False)
                        expressionYKnob.setValue('(1+2*parent.velocityVariance.y*' + str((random.random() - 0.5)) + ') * parent.startVelocity.y + 2 * ' + str((random.random() - 0.5)) + ' * parent.maxExtraVelocity.y + parent.inherentEmitterVelocity * (transformedStartPosition.y - transformedStartPosition2.y)')
                        startVelocityKnob.setExpression(expressionYKnob.value(),1)
                        
                        expressionZKnob = nuke.String_Knob('startVelocityExpressionZ','startVelocityExpressionZ')
                        transform.addKnob(expressionZKnob)
                        expressionZKnob.setVisible(False)
                        expressionZKnob.setValue('(1+2*parent.velocityVariance.z*' + str((random.random() - 0.5)) + ') * parent.startVelocity.z + 2 * ' + str((random.random() - 0.5)) + ' * parent.maxExtraVelocity.z + parent.inherentEmitterVelocity * (transformedStartPosition.z - transformedStartPosition2.z)')
                        startVelocityKnob.setExpression(expressionZKnob.value(),2)
                        
                        startVelocityKnob.setVisible(False)

                        # mass
                        massKnob = nuke.Double_Knob('mass','mass')
                        transform.addKnob(massKnob)
                        expressionKnob = nuke.String_Knob('massExpression','massExpression')
                        transform.addKnob(expressionKnob)
                        expressionKnob.setVisible(False)
                        expressionKnob.setValue('(1+2*parent.massVariance*' + str((random.random() - 0.5)) + ') * parent.mass')
                        massKnob.setExpression(expressionKnob.value())
                        massKnob.setVisible(False)

                        # store the particle number
                        particleNumberKnob = nuke.Int_Knob('particleNumber','particleNumber')
                        transform.addKnob(particleNumberKnob)
                        particleNumberKnob.setValue(curParticle)
                        particleNumberKnob.setVisible(False)

                        # store the particle root node
                        particleRootKnob = nuke.String_Knob('particleRoot','particleRoot')
                        transform.addKnob(particleRootKnob)
                        particleRootKnob.setValue('particleRoot' + str(curParticle))
                        particleRootKnob.setVisible(False)

                        # update function trigger
                        #updateTriggerKnob = nuke.Int_Knob('updateTrigger','updateTrigger')
                        #transform.addKnob(updateTriggerKnob)
                        #updateTriggerKnob.setExpression('!particleRoot' + str(curParticle) + '.alive || ')
                        #updateTriggerKnob.setVisible(False)
                        transform['pivot'].setExpression('0 * (!particleRoot' + str(curParticle) + '.alive || [ python particles.Update(nuke.thisNode(), nuke.thisParent(), nuke.frame()) ])',0)

                        # calculate scale and rotation
                        transform['uniform_scale'].setExpression('(1+2*parent.birthSizeVariance*' + str((random.random() - 0.5)) + ') * particleRoot' + str(curParticle) + '.sizeInterpolationValue * (parent.birthSize) + (1+2*parent.deathSizeVariance*' + str((random.random() - 0.5)) + ') * (1 - particleRoot' + str(curParticle) + '.sizeInterpolationValue) * (parent.deathSize)')

                        startRotation = '(1+2*parent.rotationVariance.x*' + str((random.random() - 0.5)) + ') * parent.rotation.x + 2*' + str((random.random() - 0.5)) + ' * parent.maxExtraRotation.x'
                        rotation = startRotation + ' + frame / root.fps * ((1+2*parent.rotationSpeedVariance.x*' + str((random.random() - 0.5)) + ') * parent.rotationSpeed.x + 2*' + str((random.random() - 0.5)) + ' * parent.maxExtraRotationSpeed.x)'
                        transform['rotate'].setExpression(rotation,0)
                        startRotation = '((1+2*parent.rotationVariance.y*' + str((random.random() - 0.5)) + ') * parent.rotation.y + 2*' + str((random.random() - 0.5)) + ' * parent.maxExtraRotation.y)'
                        rotation = startRotation + ' + frame / root.fps * ((1+2*parent.rotationSpeedVariance.y*' + str((random.random() - 0.5)) + ') * parent.rotationSpeed.y + 2*' + str((random.random() - 0.5)) + ' * parent.maxExtraRotationSpeed.y)'
                        transform['rotate'].setExpression(rotation,1)
                        startRotation = '((1+2*parent.rotationVariance.z*' + str((random.random() - 0.5)) + ') * parent.rotation.z + 2*' + str((random.random()) - 0.5) + ' * parent.maxExtraRotation.z)'
                        rotation = startRotation + ' + frame / root.fps * ((1+2*parent.rotationSpeedVariance.z*' + str((random.random() - 0.5)) + ') * parent.rotationSpeed.z + 2*' + str((random.random() - 0.5)) +  ' * parent.maxExtraRotationSpeed.z)'
                        transform['rotate'].setExpression(rotation,2)

                        # switch to empty object if particle not alive
                        switch = nuke.nodes.Switch(label='auto',name='enableParticle'+str(curParticle))
                        switch.setInput(0, transform)
                        #switch.setInput(1, nuke.toNode('emptyObject'))
                        switch['which'].setExpression('!particleRoot' + str(curParticle) + '.alive')

                        # add new scene if necessary
                        newSceneCount = int(curParticle / (998 + (curSceneCount == 1))) + 1
                        if (newSceneCount > curSceneCount):
                            newScene = nuke.nodes.Scene(label='auto')
                            if (scene != None):
                                newScene.setInput(0, scene)
                            scene = newScene
                            curSceneCount += 1
                            nuke.toNode('outScene').setInput(3, scene)

                        scene.setInput(curParticle % (998 + (curSceneCount == 1)) + (curSceneCount != 1), switch)
                        curParticle += 1

        nuke.root().setModified(True)
        undo.end()
    #except:
    #    nuke.message('Error while generating particles')
