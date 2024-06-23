proc exportLight {} {
	
	set f [open [knob [selected_node].outFile] w]
	
	if {[knob [selected_node].pointLight] == true} {
		
	set output "POINTLIGHT"
	set output "$output\nPositionX "
	set output "$output[knob [selected_node].posX]"
	set output "$output\nPositionY "
	set output "$output[knob [selected_node].posY]"
	set output "$output\nPositionZ "
	set output "$output[knob [selected_node].posZ]"
	
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
	
	} else {
		
	set output "DIRLIGHT"
	set output "$output\nPhi "
	set output "$output[knob [selected_node].phi]"
	set output "$output\nTheta "
	set output "$output[knob [selected_node].theta]"
	
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
	
	puts $f $output
	
	}
	close $f
}