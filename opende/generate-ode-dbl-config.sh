#!/bin/bash
# Generate a minimal ode-dbl-config script when configure is not available

cat > ode-dbl-config << 'EOF'
#!/bin/sh
# Minimal ode-dbl-config for CI builds

prefix=/usr/local
exec_prefix=${prefix}
includedir=${prefix}/include
libdir=${exec_prefix}/lib

usage()
{
    cat <<EOH
Usage: ode-dbl-config [OPTIONS]
Options:
    [--prefix]
    [--exec-prefix]
    [--version]
    [--cflags]
    [--libs]
EOH
    exit $1
}

if test $# -eq 0; then
    usage 1
fi

while test $# -gt 0; do
    case "$1" in
    -*=*) optarg=`echo "$1" | sed 's/[-_a-zA-Z0-9]*=//'` ;;
    *) optarg= ;;
    esac

    case $1 in
    --prefix=*)
        prefix=$optarg
        ;;
    --prefix)
        echo $prefix
        ;;
    --exec-prefix=*)
        exec_prefix=$optarg
        ;;
    --exec-prefix)
        echo $exec_prefix
        ;;
    --version)
        echo 0.16.2
        ;;
    --cflags)
        # Look for include directories relative to script location
        SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
        if [ -d "$SCRIPT_DIR/include/ode-dbl" ]; then
            echo "-I$SCRIPT_DIR/include/ode-dbl -DdDOUBLE"
        elif [ -d "$SCRIPT_DIR/include/ode" ]; then
            echo "-I$SCRIPT_DIR/include/ode -DdDOUBLE"
        else
            echo "-I${includedir}/ode -DdDOUBLE"
        fi
        ;;
    --libs)
        echo "-L${libdir} -lode"
        ;;
    *)
        usage 1
        ;;
    esac
    shift
done

exit 0
EOF

chmod +x ode-dbl-config
echo "Generated ode-dbl-config"