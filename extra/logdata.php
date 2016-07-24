<?php

define('TMP_FILE_PATH','/tmp/');
define('FILE_PREFIX','BrewPi_Update_');

if($_SERVER['REQUEST_METHOD'] == 'POST')
{
	$para=$_POST;
}
else
{
	$para=$_GET;	

}
	
$url = "https://script.google.com/macros/s/" .  $para["script"] . "/exec";
$data = array('bt' => $para["bt"],
			  'bs' => $para["bs"],
			  'ft' => $para["ft"],
			  'fs' => $para["fs"],
			  'ss' => $para["ss"],
			  'st' => $para["st"],
			  'pc' => $para["pc"]);
// use key 'http' even if you send the request to https://...
$options = array(
    'http' => array(
        'header'  => "Content-type: application/x-www-form-urlencoded\r\n",
        'method'  => 'POST',
        'content' => http_build_query($data)
    )
);
$context  = stream_context_create($options);
$result = file_get_contents($url, false, $context);
//if ($result === FALSE) { /* Handle error */ }

touch(TMP_FILE_PATH . FILE_PREFIX . $para["st"]);

?>