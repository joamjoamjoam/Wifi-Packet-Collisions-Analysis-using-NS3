import sys


idList = []
timeList = []



if __name__ == "__main__":
    for line in sys.stdin:
        if " manager" in line:
           managerId = line[33:35]
           time = line[40:]
           if managerId not in idList:
              idList.append(managerId);
              timeList.append(time)
           else:
              timeList[idList.index(managerId)] = time
        elif "build" in line:
           continue
        elif "[" in line:
           continue
        else:
           sys.stdout.write(line)
    totalTime = 0     
    for i in xrange(len(idList)):
        sys.stdout.write('Total time for manager %s: %s' % (idList[i],timeList[i]))
        totalTime += float(timeList[i])
    sys.stdout.write("Total BackOff Time Delay (s) %f\n" % totalTime)

