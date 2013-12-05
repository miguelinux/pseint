<?php
	$c = 0;
	while ($c<=10) {
		$c = $c+1;
		echo $c,"\n";
	}
	$c = 0;
	do {
		$c = $c+1;
		echo $c,"\n";
	} while ($c!=10);
	$c = 0;
	do {
		$c = $c+1;
		echo $c,"\n";
	} while ($c<10);
	if ($c==10) {
		echo 'Si',"\n";
	}
	if ($c==10) {
		echo 'Si',"\n";
	} else {
		echo 'No',"\n";
	}
?>
