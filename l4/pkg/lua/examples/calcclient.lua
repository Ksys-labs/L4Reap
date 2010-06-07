
function printf(...)
 io.write(string.format(...))
end

io.write("Hi world from example client in LUA\n");

server = l4.re_cap_alloc();
printf("Alloced a cap: %x\n", server);

server = l4.re_get_env_cap("calc_server");

if not l4.cap_is_valid(server) then
  printf("calc_server not found in namespace, err %d\n", res);
  return 1
end

l4.utcb_mr_put(0, 0, 9, 3);
res = l4.ipc_call(server, l4.msgtag(0, 3, 0, 0), l4.timeout_never());

printf("SUB result: %d (IPC: %x)\n", l4.utcb_mr_get(0), res);

l4.utcb_mr_put(0, 1, 8);
res = l4.ipc_call(server, l4.msgtag(0, 2, 0, 0), l4.timeout_never());

printf("NEG result: %d (IPC: %x)\n", l4.utcb_mr_get(0), res);

printf("done\n");

return 0
