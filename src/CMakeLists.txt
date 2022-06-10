add_executable(keyer
  iambic.c
  hardware.c
  main.c
  )

# Import pico_stdlib which imports commonly used features
target_link_libraries(keyer pico_stdlib)

# Create map/bin/hex file etc.
pico_add_extra_outputs(keyer)

# add url via pico_set_program_url
example_auto_set_url(keyer)
