<?php

define('TMP_FILE_PATH','/tmp/');
define('FILE_PREFIX','BrewPi_Update_');
define('MARGIN_TIME',180);

$verbosal=0;
$dryrun=0;

$CHECK_LIST=array(array('name'=>'brewpi01','period'=>300, 'label' => 'bluemoon','notify'=>'mailto:YOURNAME@YOURHOST.COM'));


/////// event notifier

function notify_down($check,$latest)
{
	global $verbosal,$dryrun;
	$subject="[Server Monitor] " . date('m/d/Y H:i:s ') . $check['name'] . " is  Down";

	$message=<<<EOF
	The BrewPiLess {$check['name']} is Down now.
	latest update time:{$latest}
		From Service Moniter

EOF;

	$action=parse_url($check['notify']);
	if($action['scheme'] == 'mailto')
	{
		$headers = 'From: YOURNAME@YOURHOST.COM' . "\r\n" .
    			'Reply-To: YOURNAME@YOURHOST.COM' . "\r\n" .
    			'X-Mailer: PHP' ;
    	
    	if($verbosal){ echo "sending:\n subject:". $subject ."\n content:\n". $message;}
    	
		if(!$dryrun){
			mail($action['path'],$subject,$message,$headers);
		}
	}
	else
	{
		die("unsupported notification action :" . $action['scheme']);
	}
}// endof notify
	

////////// main 

foreach($CHECK_LIST as $checking)
{

	$file=TMP_FILE_PATH . FILE_PREFIX . $checking["label"];
	if(is_file($file))
	{
		$stat=stat($file);
		$latest_time=date('m/d/Y H:i:s',$stat['mtime']);
		
		if($verbosal){echo "last update:".$latest_time ."\n";}
		
		if((time() -$stat['mtime']) > ($checking['period'] + MARGIN_TIME)){
			if($verbosal){echo "BrewPi ".$checking['name'] ." may be down.\n";}
			notify_down($checking,$latest_time);
		}
	}
	else
	{
		if($verbosal){echo $file ." not found!";}
	}
}
?>