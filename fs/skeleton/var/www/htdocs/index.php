<?php

$load = false;

/* check if we have a valid interval in POST */
if ($_POST["interval"] && preg_match('/^[0-9]+$/', $_POST["interval"])) {
	file_put_contents('/sys/class/onda/onda_interval', $_POST["interval"]);
}

/* check if CPU load has been requested */
if ($_POST["cpuload"] && $_POST["seconds"] && 
							preg_match('/^[0-9]+$/', $_POST["seconds"])) {
	system("/usr/bin/cpu_load.sh " . $_POST["seconds"]);
}

/* get /sys/class/onda values to display */	
$pincount = file_get_contents('/sys/class/onda/button_count', false);
$interval = file_get_contents('/sys/class/onda/onda_interval', false);

?>
<html><head><title>Configuração Onda</title></head>
<body><h1>Configuração Onda</h1>
Bot&atilde;o pressionado <?php echo $pincount; ?> vezes
<form name="input" action="index.php" method="post">
Onda intervalo: <input type="text" name="interval" value="<?php echo $interval; ?>"/><br>
<input type="checkbox" name="cpuload" value="1" />Carga na CPU
<input type="text" name="seconds" value="0"/><br>
<input type="submit" value="Submit" /></form>
</body></html>
