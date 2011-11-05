<?php

function smarty_function_msgPool($params, &$smarty)
{
	if(class_available("msgPool") && isset($params['type'])){
		$parameter = array();
		foreach($params as $para => $value){
			if(!preg_match("/^type$/i",$para)){
				$parameter[$para] = $value;
			}
		}
		if(is_callable("msgPool::".$params['type'])){
			echo call_user_func_array(array("msgPool",$params['type']), $parameter);
		}else{
			trigger_error("Unknown msgPool function ".$params['type']);
		}
	}else{
		trigger_error("Unknown class msgPool.");
	}
}
?>
