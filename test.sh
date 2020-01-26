#/bin/bash



for i in tracefiles/*.trf
do
		if [ $(echo $i | grep "p1") ]; then
			continue;
		fi

		echo "test_$i"
done
