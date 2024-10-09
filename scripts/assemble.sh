#!/bin/bash

assemble() {
  input="$1"

  # convert c source to assemble the input with cc
  cc -S -o assemble.s $input
  cat assemble.s
}

# assemble ./samples/0.c