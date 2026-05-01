valgrind --tool=massif ./cg0
massif-visualizer $(ls -t massif.out.* | head -n1)
rm -rf ./massif.out.*

