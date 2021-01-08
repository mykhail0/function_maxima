#!/bin/bash
valgrind --error-exitcode=1 --errors-for-leak-kinds=definite --leak-check=full --quiet
