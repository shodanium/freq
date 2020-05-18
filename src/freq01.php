#!/usr/bin/php
<?php

$fp = fopen($argv[1], "r") or die("fopen-input");
$fp2 = fopen($argv[2], "wb+") or die("fopen-output");
$h = array();
while ($l = fgets($fp))
{
	$m = array();
	if (preg_match_all("/[a-zA-Z]+/", $l, $m))
		foreach ($m[0] as $w)
			@$h[strtolower($w)]++;
}

$fold = array();
foreach ($h as $t => $f)
	$fold[] = array($f, $t);
usort($fold, function($a, $b) {
	if ($a[0] == $b[0])
		return $a[1] <=> $b[1];
	return $b[0] <=> $a[0];
});
foreach ($fold as $e)
	fwrite($fp2, "$e[0] $e[1]\r\n");
