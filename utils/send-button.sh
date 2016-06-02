#!/bin/sh
$(cd $(dirname $0); pwd)/send-client-event.sh '[ { "type": "microbit_button", "data": { "id": '$1', "state": '$2' } } ]'
