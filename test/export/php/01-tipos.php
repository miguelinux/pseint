<?php
	$stdin = fopen('php://stdin','r');
	settype($a,'integer');
	$a = 1;
	$b = 2.5;
	$c = true;
	$d = false;
	$e = 'Hola';
	$f = 'Hola Mundo';
	$p=rtrim(fgets($stdin),"\n");
	echo $p,"\n";
?>
