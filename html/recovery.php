<?php

require_once("../include/php_setup.inc");
require_once("functions.inc");
require_once("variables.inc");

$pwRecovery = new passwordRecovery();

$pwRecovery->execute();

$pwRecovery->displayPWchanger();

?>
