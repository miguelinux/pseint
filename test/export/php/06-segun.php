<?php
	$stdin = fopen('php://stdin','r');
	fscanf($stdin,"%f",$c);
	switch ($c) {
	case 1:
		echo '1',"\n";
		break;
	case 3: case 5: case 9:
		echo '3, 5 o 9',"\n";
		break;
	case 7:
		echo '7',"\n";
		break;
	default:
		echo 'ni 1, ni 3, ni 5, ni 7, ni 9',"\n";
		break;
	}
?>
