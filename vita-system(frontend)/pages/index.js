import Head from 'next/head'
import Geral from '../src/components/geral/geral'
import SensorData from '../src/components/teste/teste'
import Topbar from '../src/components/Topbar/Topbar'


/* import Image from 'next/image'
import styles from '../styles/Home.module.css' */

export default function Home() {
  return (
    <div >
      <Head>
        <title>VITA | Respirador inteligente</title>
        <meta name="description" content="Generated by create next app" />
        <link rel="icon" href="/favicon.ico" />
        <link rel="stylesheet" href="style.css"/>    
        <link href="dist/hamburgers.css" rel="stylesheet"/>
        
      </Head>

{/*       style={{ backgroundColor: '#000080', color: '#fff' }} */}
      <main >
        <Topbar />
        <SensorData/>
        <Geral/>
            
      </main>
    </div>
  )
}