pipeline {
  agent any
  triggers {
    pollSCM('H/15 7-20 * * *')
  }
  stages {
    stage('Build') {
      steps {
        sh 'git submodule update --init --recursive vardb'
        sh 'scons'
      }
    }
  }
  post {
    failure {
      mail(to: 'cjw@ucar.edu janine@ucar.edu cdewerd@ucar.edu taylort@ucar.edu', subject: 'oap Jenkinsfile build failed', body: 'oap Jenkinsfile build failed')
    }
  }
  options {
    buildDiscarder(logRotator(numToKeepStr: '10'))
  }
}
