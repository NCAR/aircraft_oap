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
    success {
      mail(to: 'cjw@ucar.edu janine@ucar.edu cdewerd@ucar.edu taylort@ucar.edu', subject: 'oap Jenkinsfile build successful', body: 'oap Jenkinsfile build successful')
    }
  }
  options {
    buildDiscarder(logRotator(numToKeepStr: '10'))
  }
}
