import React, { useState } from "react";
import Geral from "../src/components/geral/geral";
import handleLogin from "../src/context/AuthContext";

const Login = () => {
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");
  async function login(){
    handleLogin(email,password)
  }

  return (
    <>
    <div>
      <h1>Login</h1>
      <input
        type="text"
        placeholder="Email"
        value={email}
        onChange={(e) => setEmail(e.target.value)}
      />
      <input
        type="password"
        placeholder="Senha"
        value={password}
        onChange={(e) => setPassword(e.target.value)}
      />
      <button onClick={login}>Login</button>
    </div>
   
    <Geral/>
    </>
  );
};

export default Login;
