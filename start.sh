#!/bin/bash
CWD=$(cd "$(dirname $0)";pwd)
$CWD/easydarwin install
$CWD/easydarwin start 