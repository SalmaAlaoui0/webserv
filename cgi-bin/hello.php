#!/usr/bin/env php
<?php
// If you're using CGI, make sure this file is executable (chmod +x filename.php)

// Send HTTP header (required for CGI)
header("Content-Type: text/html");

// Start HTML output
echo "<!DOCTYPE html>";
echo "<html lang='en'>";
echo "<head>";
echo "<meta charset='UTF-8'>";
echo "<title>PHP Script Test</title>";
echo "</head>";
echo "<body>";

echo "<h1>✅ PHP Script is Working!</h1>";
echo "<p>Current Server Time: <strong>" . date("Y-m-d H:i:s") . "</strong></p>";
echo "<p>Your IP: <strong>" . $_SERVER['REMOTE_ADDR'] . "</strong></p>";

// Check GET parameters
if (!empty($_GET)) {
    echo "<h2>GET Parameters:</h2><pre>";
    print_r($_GET);
    echo "</pre>";
}

// Check POST parameters
if (!empty($_POST)) {
    echo "<h2>POST Parameters:</h2><pre>";
    print_r($_POST);
    echo "</pre>";
}