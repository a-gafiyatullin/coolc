#!/bin/bash

$3/coolc $4 $1
str=$1
$2/spim "${str/'.cl'/'.s'}"