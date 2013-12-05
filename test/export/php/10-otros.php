<?php
	$stdin = fopen('php://stdin','r');
	echo "\n";
	fgetc($stdin);
	usleep(100*1000);
	sleep(1);
	echo M_PI,"\n";
?>
