<?php
header("Content-type:application/json");
$exe = "./phpface";
switch($_POST["func"]){
	case "version":
		exec("$exe -v", $output, $return_val);
	break;
	case "active":
		exec("$exe -a", $output, $return_val);
	break;
	case "detect":
		
		$tmp = $_FILES['image']['tmp_name'];
		
		exec("$exe -d $tmp", $output, $return_val);
		
	break;
	default:
		$output = ['{"error_code":100006,"message":"invalid parameter"}'];
}

echo $output[0];
?>
