WARNING: Needs to compile the executable 'csr2coo' first before using turnt for testing.

With TURNT we have that each input file is a test.
TURNT does snapshot testing, i.e. it checks whether given the same input the output
matches with the snapshot of a previous output.

To take a snapshot of the "golden" outputs from the input files.
===
turnt --save samples/*.mm
turnt --save samples/*.mtx
===

To run all tests
===
turnt samples/*.mm
turnt samples/*.mtx
===

if there is a mismatch and see the changes
===
turnt --diff
===

FOR MORE DETAILS: https://github.com/cucapra/turnt#configuring
