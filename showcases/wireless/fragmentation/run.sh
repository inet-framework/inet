#!/bin/bash

inet -u Cmdenv -c DCFnofrag &
inet -u Cmdenv -c DCFfrag &
inet -u Cmdenv -c HCFfrag &
inet -u Cmdenv -c HCFfragblockack
