proc exportSpotLight {} {
	
	set f [open [knob [selected_node].outFile] w]
	
	set output "SPOTLIGHT"
	set output "$output\nPositionX "
	set output "$output[knob [selected_node].posX]"
	set output "$output\nPositionY "
	set output "$output[knob [selected_node].posY]"
	set output "$output\nPositionZ "
	set output "$output[knob [selected_node].posZ]"
	
	set output "$output\nTargetPositionX "
	set output "$output[knob [selected_node].TPosX]"
	set output "$output\nTargetPositionY "
	set output "$output[knob [selected_node].TPosY]"
	set output "$output\nTargetPositionZ "
	set output "$output[knob [selected_node].TPosZ]"
	
	set output "$output\nConeAngle "
	set output "$output[knob [selected_node].coneAngle]"
	set output "$output\nConeDeltaAngle "
	set output "$output[knob [selected_node].coneDeltaAngle]"
	set output "$output\nBeamDistribution "
	set output "$output[knob [selected_node].beamDistribution]"
	
	
	set output "$output\nIntensity "
	set output "$output[knob [selected_node].intensity]"
	set output "$output\nColorR "
	set output "$output[knob [selected_node].colorR]"
	set output "$output\nColorG "
	set output "$output[knob [selected_node].colorG]"
	set output "$output\nColorB "
	set output "$output[knob [selected_node].colorB]"
	if {[knob [selected_node].specular] == true} {
	set output "$output\nRoughness "
	set output "$output[knob [selected_node].roughness]"	
	}
	if {[knob [selected_node].attenuate] == true} {
	set output "$output\nAttenuate"
	set output "$output Yes"
	}
	
	puts $f $output
	
	close $f
}