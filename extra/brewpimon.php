<?php

define('FROM_EMAIL' ,'YOUREMAIL@gmail.com');
define('TO_EMAIL' ,'YOUREMAIL@gmail.com');

define('TMP_FILE_PATH','/tmp/');
define('FILE_PREFIX','BrewPi_Update_');
define('MARGIN_TIME',180);

$verbosal=0;
$dryrun=0;

$CHECK_LIST=array(array('name'=>'brewpi01','period'=>300, 'label' => 'bluemoon','notify'=>'mailto:' . TO_EMAIL));


/////// event notifier

function report($notify,$name,$state,$latest_time)
{
	global $verbosal,$dryrun;

	$subject="[Server Monitor] " . date('m/d/Y H:i:s ') . $name . " is " . $state;

	$message=<<<EOF
	The BrewPiLess {$name} is {$state} now.
	latest update time: {$latest_time}
		From Service Moniter

EOF;

	$action=parse_url($notify);
	if($action['scheme'] == 'mailto')
	{
		$headers = 'From: ' .FROM_EMAIL . "\r\n" .
    			'Reply-To: ' .FROM_EMAIL.  "\r\n" .
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
	
define('SERVER_UNKNOWN',0);
define('SERVER_ALIVE',1);
define('SERVER_DOWN',2);

class ServerStateKeeper
{
	var $server_name;
	var $serer_state;

	function ServerStateKeeper($servername)
	{
		$this->server_name=$servername;
		$alive=FALSE;
		$down=FALSE;
		
		if(is_file($this->down_file()))
		{
			$down=TRUE;
		}
		if(is_file($this->alive_file()))
		{
			$alive=TRUE;
		}
		
		if($down && $alive)
		{
			$stat=stat($this->alive_file());
			// error case, change to unknown
			unlink($this->down_file());
			unlink($this->alive_file());
			$this->server_state = SERVER_UNKNOWN;			
		}
		else if($down)
		{
			$this->server_state = SERVER_DOWN;
		}
		else if($alive)
		{
			$this->server_state = SERVER_ALIVE;
		}
		else
		{
			$this->server_state = SERVER_UNKNOWN;
		}	
	}

	function down_file()
	{
		return  TMP_FILE_PATH .'server_monitor.' . $this->server_name . '.down';
	}
	function alive_file()
	{
		return  TMP_FILE_PATH .'server_monitor.' . $this->server_name . '.alive';
	}
	
	function isUnknown()
	{
		return ($this->server_state == SERVER_UNKNOWN);
	}
	
	function isDown()
	{
		return ($this->server_state == SERVER_DOWN);	
	}
	
	function isAlive()
	{
		return ($this->server_state == SERVER_ALIVE);
	}
	
	function latest_time()
	{
		if(is_file($this->alive_file()))
		{
			$stat=stat($this->alive_file());
			return $stat['mtime'];
		}
		else if(is_file($this->down_file()))
		{
			$stat=stat($this->down_file());
			return $stat['mtime'];
		}
	
		return 0;
	}
	
	function set_server_down()
	{
		if($this->isAlive())
		{
			unlink($this->alive_file());
		}
		touch($this->down_file());
		$this->server_state=SERVER_DOWN;		
	}
	
	function set_server_alive()
	{
		if($this->isDown())
		{
			unlink($this->down_file());
		}
		touch($this->alive_file());
		$this->server_state=SERVER_ALIVE;
	}
	
	function set_server_unknown()
	{
		if($this->isUnknown())
		{
			return;
		}
		if($this->isDown())
		{
			unlink($this->down_file());
		}

		if($this->isAlive())
		{
			unlink($this->alive_file());
		}
		
		$this->server_state=SERVER_UNKNOWN;
	}
}

////////// main 

foreach($CHECK_LIST as $checking)
{

	$file=TMP_FILE_PATH . FILE_PREFIX . $checking["label"];
		
	if(is_file($file))
	{
		$serverstate=new ServerStatekeeper($checking['name']);

		$stat=stat($file);
		$latest_time=date('m/d/Y H:i:s',$stat['mtime']);
		
		if($verbosal){echo "last update:".$latest_time ."\n";}
		
		if((time() -$stat['mtime']) > ($checking['period'] + MARGIN_TIME)){
			if($verbosal){echo "BrewPi ".$checking['name'] ." may be down.\n";}

			if(!$serverstate->isDown()){
				report($checking['notify'],$checking['name'],"DOWN",$latest_time);
				$serverstate->set_server_down();
			}
		}else{
			if(!$serverstate->isAlive()){
				report($checking['notify'],$checking['name'],"ALIVE",$latest_time);
				$serverstate->set_server_alive();
			}			
		}
	}
	else
	{
		if($verbosal){echo $file ." not found!";}
	}
}
?>