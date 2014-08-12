#!/usr/bin/env bash

# Use EXP as default, if it exists.
if [[ ! -z ${EXP} ]]; then
  SUFFIX=$(echo $EXP | cut -d, -f2).$(echo $EXP | cut -d, -f1).isi.deterlab.net
fi

# parse args.
while getopts ":s:c:o:n:p:e:vh" opt; do
  case $opt in
    s) SERVERS=$OPTARG ;;
    c) CLIENTS=$OPTARG ;;
    n) NODES=$OPTARG ;;
    p) APP_PATH=$OPTARG ;;
    e) SUFFIX=$(echo $OPTARG | cut -d, -f2).$(echo $OPTARG | cut -d, -f1).isi.deterlab.net ;;
    v) VERBOSE=1 ;;
    h|?|*)
      echo $(basename $0) [args] command
      echo -e "\t-s servers   : The number of servers in the experiment."
      echo -e "\t-c clients   : The number of clients in the experiment."
      echo -e "\t-n nodes     : The number of clients on client machines."
      echo -e "\t-p path      : The path to dissent binary on users."
      echo -e "\t-e exp       : Experiment name in the form PID,EID."
      exit 1
  esac
done

# skip read args
shift $((OPTIND-1))

# default values, but allow to be cahnged by the environment or command line.
SUFFIX=${SUFFIX:-mnservers.SAFER.isi.deterlab.net}
SERVERS=${SERVERS:-2}
CLIENTS=${CLIENTS:-2}
NODES=${NODES:-1}
APP_PATH=${APP_PATH:-/users/davidiw/dissent}

# hard coded can change if needed.
CLIENT="user"
SERVER="server"
BASE_PATH="/local/logs/run"
BASE_PORT="31000"
BASE_WEB_PORT="30100"
SERVER_IP="10.0.1"
CLIENT_IP="10.0.0"
DISSENT_BASE_PARAMS="--path_to_private_keys=$BASE_PATH/keys/private \
  --path_to_public_keys=$BASE_PATH/keys/public \
  --auth=true \
  --round_type=neff/csdcnet \
  --log=stdout \
  "

SSH_OPTS="-o StrictHostKeyChecking=no \
  -o HostbasedAuthentication=no \
  -o CheckHostIP=no \
  -o ConnectTimeout=10 \
  -o ServerAliveInterval=30 \
  -o BatchMode=yes \
  -o UserKnownHostsFile=/dev/null"

if [[ ! -z ${VERBOSE} ]]; then
    echo "Running script with configuration:"
    echo "SERVERS=${SERVERS}"
    echo "CLIENTS=${CLIENTS}"
    echo "SUFFIX=${SUFFIX}"
    echo "APP_PATH=${APP_PATH}"
fi

ssh_exec()
{
  result="$(ssh -f $SSH_OPTS $@ <&- 2>&1)"
  if [[ $(echo $result | grep -v Permanently) == "" ]]; then
    return
  fi
  echo "Failed: "$result" for "$@
}

scp_exec()
{
  result="$(scp $SSH_OPTS $@ <&- 2>&1)"
  if [[ $(echo $result | grep -v Permanently) == "" ]]; then
    return
  fi
  echo "Failed: "$result" for "$@
}

logs()
{
  mkdir logs
  for (( i = 0 ; i < $SERVERS ; i = i + 1 )); do
    scp_exec "$SERVER-$i:$BASE_PATH/log.* logs/." &
  done

  for (( i = 0 ; i < $CLIENTS ; i = i + 1 )); do
    scp_exec "$CLIENT-$i:$BASE_PATH/log.* logs/." &
  done
  wait
}

setup()
{
  cd $APP_PATH &> /dev/null
  tar -cjf dissent.tbz dissent keys
  cd - &> /dev/null
  cmd="rm -rf $BASE_PATH; \
      mkdir -p $BASE_PATH; \
      cp $APP_PATH/dissent.tbz $BASE_PATH/.; \
      cd $BASE_PATH; \
      tar -jxf dissent.tbz"

  for (( i = 0 ; i < $SERVERS ; i = i + 1 )); do
    echo "$SERVER-$i"
    ssh_exec $SERVER-$i $cmd &
  done

  for (( i = 0 ; i < $CLIENTS ; i = i + 1 )); do
      echo "$CLIENT-$i"
      ssh_exec $CLIENT-$i $cmd &
  done

  wait
}

start()
{
  for key in $(ls $BASE_PATH/keys/private); do
    keys[$count]=$key
    count=$((count+1))
  done

  server_ids=""
  server_ips=""
  for (( i = 0 ; i < $SERVERS ; i = i + 1 )); do
    server_ids=$server_ids" --server_ids=${keys[$i]}"
    server_ips=$server_ips" --remote_endpoints=tcp://$SERVER_IP.$((1 + $i)):$BASE_PORT"
  done
  clients_base=$(($CLIENTS * $NODES / $SERVERS))

  for (( i = 0 ; i < $SERVERS ; i = i + 1 )); do
    ip="$SERVER_IP.$((1 + $i))"
    id=${keys[$i]}
    server=$ip:$BASE_PORT
    server_ip[$i]=$server

    clients=$clients_base
    if [[ $(($CLIENTS * $NODES % $SERVERS)) -gt $i ]]; then
      clients=$(($clients + 1))
    fi

    params=$DISSENT_BASE_PARAMS" \
      --local_id=$id \
      --local_endpoints=tcp://$server \
      $server_ips \
      $server_ids \
      --web_server_url=http://127.0.0.1:$BASE_WEB_PORT \
      --minimum_clients=$clients
      "

    echo "$SERVER-$i"
    echo "$SERVER$i,$ip:$BASE_PORT,$id" >> $BASE_PATH/ids
    ssh_exec $SERVER-$i.$SUFFIX "sudo bash -c 'ulimit -n 65536;
      cd $BASE_PATH;\
      ((./dissent $params &> log.$SERVER.$i)&)'" &
  done
  wait

  for (( i = 0 ; i < $CLIENTS; i = i + 1 )); do
    ip="$CLIENT_IP.$((1 + $i))"

    for (( j = 0 ; j < $NODES; j = j + 1 )); do
      index=$(($i *$NODES + $j))
      server=${server_ip[$(($index % $SERVERS))]}
      id=${keys[$(($index + $SERVERS))]}
      port=$(($BASE_PORT + $j))
      web_port=$(($BASE_WEB_PORT + $j))

      client_line=""


      params=$DISSENT_BASE_PARAMS" \
        --local_id=$id \
        --local_endpoints=tcp://$ip:$port \
        --remote_endpoints=tcp://$server \
        --web_server_url=http://127.0.0.1:$web_port \
        $server_ids
        "

        echo "$CLIENT$i.$j,$ip:$port,$id" >> $BASE_PATH/ids
      if [[ $i -eq 0 && $j -eq 0 ]]; then
        client_line=$client_line" (($BASE_PATH/dissent $params &> $BASE_PATH/log.$CLIENT.$i.$j)&);"
      else
        client_line=$client_line" (($BASE_PATH/dissent $params &> /dev/null)&);"
      fi
    done

    echo "$CLIENT-$i"
    ssh_exec $CLIENT-$i "$client_line" &
  done
  wait
}

stop()
{
  for (( i = 0 ; i < $SERVERS ; i = i + 1 )); do
    echo "$SERVER-$i"
    ssh_exec $SERVER-$i "sudo bash -c 'pkill -KILL dissent'" &
  done

  for (( i = 0 ; i < $CLIENTS ; i = i + 1 )); do
    echo "$CLIENT-$i"
    ssh_exec $CLIENT-$i "sudo bash -c 'pkill -KILL dissent'" &
  done

  wait
}

funct=$1
$funct ${@:2}
