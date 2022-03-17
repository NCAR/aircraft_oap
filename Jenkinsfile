pipeline {
  agent any
  triggers {
    pollSCM('H/15 7-20 * * *')
  }
  stages {
    stage('Checkout Scm') {
      steps {
        git 'eolJenkins:ncar/aircraft_oap'
      }
    }

    stage('Build') {
      steps {
        sh 'scons'
      }
    }

  }
  post {
    success {
      mail(to: 'cjw@ucar.edu janine@ucar.edu cdewerd@ucar.edu taylort@ucar.edu', subject: 'oap Jenkinsfile build successful', body: 'oap Jenkinsfile build successful')
    }

  }
  options {
    buildDiscarder(logRotator(numToKeepStr: '10'))
  }
}
