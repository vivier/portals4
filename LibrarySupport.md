If you have a library which supports Portals 4 and is not listed, please let us know.

# Implementations #

## Open MPI ##

  * You'll need either a checkout of the SVN trunk or a nightly tarball:
```
svn co https://svn.open-mpi.org/svn/ompi/trunk
cd trunk
./autogen.sh

OR

http://www.open-mpi.org/nightly/trunk/
```

  * Then configure Open MPI:
```
   ./configure --prefix=<PREFIX> \
      --with-portals4=<PORTALS4> \
      --with-platform=contrib/platform/snl/portals4-orte
   make all install
```


The implementation is still a work in progress and likely to die in strange ways.  The one-sided and collective algorithms have not been optimized for Portals 4 at this time.

## Portals SHMEM ##

  * An implementation of the OpenSHMEM specification over Portals 4 is available on [[Google Code](http://code.google.com/p/portals-shmem/)].