<?php
	$stdin = fopen('php://stdin','r');
	settype($a,'integer');
	settype($b,'float');
	settype($c,'string');
	settype($d,'boolean');
	fscanf($stdin,"%d",$a);
	fscanf($stdin,"%f",$b);
	$c=rtrim(fgets($stdin),"\n");
	fscanf($stdin,"%d",$d);
	echo $a,"\n";
	echo $b,"\n";
	echo $c,"\n";
	echo $d,"\n";
	fscanf($stdin,"%d",$a);
	fscanf($stdin,"%f",$b);
	$c=rtrim(fgets($stdin),"\n");
	fscanf($stdin,"%d",$d);
	echo $a,$b,$c,$d,"\n";
	echo 'Esta linea no lleva enter al final';
?>
