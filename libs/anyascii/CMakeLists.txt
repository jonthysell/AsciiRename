cmake_minimum_required(VERSION 3.16.0)
project(anyascii C)

add_library(anyascii anyascii.h anyascii.c utf8.h utf8.c)
