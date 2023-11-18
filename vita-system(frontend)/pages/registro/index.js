import React, { useState } from 'react';
import Router from 'next/router';
import styles from './Signup.module.css';

const Signup = () => {
  const [signupError, setSignupError] = useState('');
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [passwordConfirmation, setPasswordConfirmation] = useState('');
  const [nome, setNome] = useState('');
  const [nasc, setNasc] = useState('');
  const [natural, setNatural] = useState('');
  const [cel, setCel] = useState('');
  const [telefone, setTelefone] = useState('');
  const [endereco, setEndereco] = useState('');
  const [cidade, setCidade] = useState('');
  const [sexo, setSexo] = useState('Masculino');

  function handleSubmit(e) {
    e.preventDefault();
    fetch('/api/users', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        email,
        password,
        nome,
        nasc,
        natural,
        cel,
        telefone,
        endereco,
        cidade,
        sexo,
      }),
    })
      .then((r) => r.json())
      .then((data) => {
        if (data && data.error) {
          setSignupError(data.message);
        }
        if (data && data.token) {
          //set cookie
          cookie.set('token', data.token, { expires: 2 });
          Router.push('/');
        }
      });
  }

  return (
    <div className={styles.signupContainer}>
      <header className={styles.header}>
        <p>Formulário</p>
      </header>
      <div className={styles.formContainer}>
        <form onSubmit={handleSubmit} className={styles.form}>
          <div className={styles.formGroup}>
            <label htmlFor="nome">Nome Completo</label>
            <input
              id="nome"
              value={nome}
              onChange={(e) => setNome(e.target.value)}
            />
          </div>
          <div className={styles.formGroup}>
            <label htmlFor="email">E-mail</label>
            <input
              id="email"
              type="email"
              value={email}
              onChange={(e) => setEmail(e.target.value)}
            />
          </div>
          <div className={styles.formGroup}>
            <label htmlFor="nasc">Data de Nascimento</label>
            <input
              id="nasc"
              type="date"
              value={nasc}
              onChange={(e) => setNasc(e.target.value)}
            />
          </div>
          <div className={styles.formGroup}>
            <label htmlFor="natural">Naturalidade</label>
            <input
              id="natural"
              value={natural}
              onChange={(e) => setNatural(e.target.value)}
            />
          </div>
          <div className={styles.formGroup}>
            <label htmlFor="cel">Celular</label>
            <input
              id="cel"
              type="number"
              value={cel}
              onChange={(e) => setCel(e.target.value)}
            />
          </div>
          <div className={styles.formGroup}>
            <label htmlFor="telefone">Telefone</label>
            <input
              id="telefone"
              type="number"
              value={telefone}
              onChange={(e) => setTelefone(e.target.value)}
            />
          </div>
          <div className={styles.formGroup}>
            <label htmlFor="endereco">Endereço</label>
            <input
              id="endereco"
              value={endereco}
              onChange={(e) => setEndereco(e.target.value)}
            />
          </div>
          <div className={styles.formGroup}>
            <label htmlFor="cidade">Cidade</label>
            <input
              id="cidade"
              value={cidade}
              onChange={(e) => setCidade(e.target.value)}
            />
          </div>
          <div className={styles.formGroup}>
            <label>Sexo</label>
            <div className={styles.radioGroup}>
              <label>
                <input
                  type="radio"
                  value="Masculino"
                  checked={sexo === 'Masculino'}
                  onChange={() => setSexo('Masculino')}
                />
                Masculino
              </label>
              <label>
                <input
                  type="radio"
                  value="Feminino"
                  checked={sexo === 'Feminino'}
                  onChange={() => setSexo('Feminino')}
                />
                Feminino
              </label>
              <label>
                <input
                  type="radio"
                  value="Outro"
                  checked={sexo === 'Outro'}
                  onChange={() => setSexo('Outro')}
                />
                Outro
              </label>
            </div>
          </div>
          <div className={styles.formGroup}>
            <button type="submit" className={styles.submitButton}>
              Cadastrar
            </button>
          </div>
        </form>
      </div>
    </div>
  );
};

export default Signup;